

class NeedMoreDataException(Exception):
    pass

class JsonStreamParserCore:
    def __init__(self):
        self.unescape_escaped = True
        self.nested_parser = None
        self.state = None
        self.buffer = ""
        self.skipping = ""
        self.finalized = False
        self.field_name = None
        self.field_value = None

    def sinkJsonData(self, data):
        self.buffer = self.buffer + data
        while True:
            try:
                done = self._streamParse()
                if done:
                    self.finalized = True
                    return self.buffer
                if self.buffer == '':
                    break
            except NeedMoreDataException:
                return None
        return None

    def _streamParse(self):
        if self.state == None:
            if self._skipTo('{') == None:
                return False
            self.state = '{'
        if self.state == '{':
            if self._skipTo('"') == None:
                return False
            self.state = '{>'
        if self.state == '{>':
            skipped = self._skipTo('"')
            if skipped == None:
                return False
            self.field_name = skipped[:-1].strip()
            self.state = '>:'
        if self.state == '>:':
            if self._skipTo(':') == None:
                return False
            self.state = ':'
        if self.state == ':':
            if not self._skipWS():
                return False
            self.state = '^'
        if self.state == '^':
            c = self.buffer[0]
            if c == '{':
                self.buffer = self.buffer[1:]
                self.state = '^>{'
            elif c == '"':
                self.buffer = self.buffer[1:]
                self.state = '^>"'
            else:
                self.state = '^>'
        if self.state == '^>{':
            json_data = self.buffer
            if self.nested_parser == None:
                #self.nested_parser = JsonStreamParser(self.callback, self.field_name)
                self.nested_parser = JsonStreamParserCore()
                self.nested_parser.onReceived = lambda field_name, field_value : self.onReceived(self.field_name + "." + field_name, field_value)
                json_data = "{" + json_data
            self.buffer = ""
            rest = self.nested_parser.sinkJsonData(json_data)
            if rest == None:
                return False
            self.nested_parser = None
            self.buffer = rest
            self.state = '$'
        if self.state == '^>"':
            skipped = self._skipTo('"', True)
            if skipped == None:
                return False
            self.field_value = skipped[:-1].strip()
            self._submit()
            self.state = '$'
        if self.state == '^>' or self.state == '$':
            done = False
            sep_idx = self._scanTo(',')
            close_idx = self._scanTo('}')
            if sep_idx != -1 and (close_idx == -1 or sep_idx < close_idx):
                skipped = self._skipTo(',')
            else:
                skipped = self._skipTo('}')
                done = True
            if skipped == None:
                return False
            if self.state == '^>':
                self.field_value = skipped[:-1].strip()
                self._submit()
            self.state = '{'
            return done
        return False

    def _submit(self):
        self.onReceived(self.field_name, self.field_value)

    def onReceived(self, field_name, field_value):
        pass

    def _skipWS(self):
        bufLen = len(self.buffer)
        for i in range(bufLen):
            c = self.buffer[i]
            if not c.isspace():
                self.buffer = self.buffer[i:]
                return True
        self.buffer = ""
        return False

    def _skipTo(self, what, allow_escape = False):
        return self.__skip(what, True, allow_escape)

    def _skipUntil(self, what, allow_escape = False):
        return self.__skip(what, False, allow_escape)

    def __skip(self, what, inclusive, allow_escape = False):
        i = self._scanTo(what, allow_escape)
        if i == -1:
            self.skipping += self.buffer
            self.buffer = ""
            return None
        # if i == -999:
        #     return None
        if inclusive:
            skipped = self.skipping + self.buffer[0:i+1]
            self.buffer = self.buffer[i+1:]
        else:
            skipped = self.skipping + self.buffer[0:i]
            self.buffer = self.buffer[i:]
        self.skipping = ""
        return skipped

    def _scanTo(self, what, allow_escape = False):
        ori_buffer = self.buffer
        bufLen = len(self.buffer)
        escaping = False
        i = 0
        max_i = bufLen
        while i < max_i:
            c = self.buffer[i]
            if escaping:
                escaping = False
                if self.unescape_escaped:
                    self.buffer = self.buffer[0:i-1] + self.buffer[i:]
                    i -= 1
                    max_i -= 1
            else:
                if allow_escape and c == '\\':
                    escaping = True
                elif c == what:
                    return i
            i += 1
        if escaping:
            self.buffer = ori_buffer
            raise NeedMoreDataException()
        else:
            return -1

class JsonStreamParser(JsonStreamParserCore):
    def __init__(self, callback):
        super().__init__()
        self.callback = callback
    def onReceived(self, field_name, field_value):
        self.callback(field_name, field_value)




    #########################################################################

import random


class JsonStreamParserTester():
    def __init__(self):
        self.json1 = ' { "int" : 123 , "int2" : 999 , "str" : "str value" , "str2" : "str value 2" , "Ftrue" : true, "Ffalse" : false , "Fnull" : null, "end":"END" }'
        self.expected1 = {'int': '123', 'int2': '999', 'str': 'str value', 'str2': 'str value 2', 'Ftrue': 'true', 'Ffalse': 'false', 'Fnull': 'null', 'end': 'END'}
        self.json2 = '{"NESTED":{"str":"str value","int":123},"INT":4321,"NESTED2":{"str2":"str value2"},"STR":"STR VALUE"}'
        self.expected2 = {'NESTED.str': 'str value', 'NESTED.int': '123', 'INT': '4321', 'NESTED2.str2': 'str value2', 'STR': 'STR VALUE'}
        self.json3 = '{ "str1": "str\\\\ing\\"1\\"", "int": 123, "str2" : "\\"string2\\"" }'
        self.expected3 = {'str1': 'str\\ing"1"', 'int': '123', 'str2': '"string2"'}

    def testIt(self):
        self._testDebug()
        self._testIt(self.json1, self.expected1)
        self._testIt(self.json2, self.expected2)
        self._testIt(self.json3, self.expected3)

    def _testIt(self, json, expected_value):
        self._testSimple(json, expected_value)
        self._testPieceWise(json, expected_value)

    def _testDebug(self):
        expected_value = {'str1': 'str\\ing"abc"-def'}
        values = {}
        parser = JsonStreamParser(lambda id, val: self._submit("S", values, id, val))
        parser.sinkJsonData('{ "str1":"str')
        parser.sinkJsonData('\\\\ing\\')
        parser.sinkJsonData('"abc\\"-def"}')
        if expected_value != values:
            print("XXX D XXX -- " + str(values))
            assert False



    def _testSimple(self, json, expected_value):
        values = self._runSimple(json)
        if expected_value != values:
            print("XXX S XXX -- " + str(values))
            assert False

    def _testPieceWise(self, json, expected_value):
        values = self._runPieceWise(json)
        if expected_value != values:
            print("XXX C XXX -- " + str(values))
            assert False

    def _runSimple(self, json):
        values = {}
        parser = JsonStreamParser(lambda id, val: self._submit("S", values, id, val))
        parser.sinkJsonData(json)
        assert parser.finalized
        return values

    def _runPieceWise(self, json):
        values = {}
        parser = JsonStreamParser(lambda id, val: self._submit("C", values, id, val))
        json_data = ""
        for c in json:
            json_data += c
            if random.random() > 0.5:
                parser.sinkJsonData(json_data)
                json_data = ""
        if json_data != "":
            parser.sinkJsonData(json_data)
        assert parser.finalized
        return values

    def _submit(self, type, values, id, val):
        if True:
            print(type + " -- '" + id + "' = '" + val + "'")
        values[id] = val

if __name__ == "__main__":
    JsonStreamParserTester().testIt()


