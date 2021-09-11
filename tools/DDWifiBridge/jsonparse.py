


class JsonStreamParser:
    '''
    :param callback
      input: element id, element valud
    '''
    def __init__(self, callback, parent_field_name = None):
        self.callback = callback
        self.parent_field_name = parent_field_name
        self.nested_parser = None
        self.state = None
        self.buffer = ""
        self.skipping = ""
        self.field_name = None
        self.field_value = None

    def sinkJsonData(self, data):
        self.buffer = self.buffer + data
        while True:
            if self.buffer == '':
                break
            done = self._streamParse()
            if done:
                return self.buffer
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
                self.nested_parser = JsonStreamParser(self.callback, self.field_name)
                json_data = "{" + json_data
            self.buffer = ""
            rest = self.nested_parser.sinkJsonData(json_data)
            if rest == None:
                return False
            self.nested_parser = None
            self.buffer = rest
            self.state = '$'
        if self.state == '^>"':
            skipped = self._skipTo('"')
            if skipped == None:
                return False
            self.field_value = skipped[:-1].strip()
            self._submit()
            self.state = '$'
        # if self.state == '^>':
        #     done = False
        #     if self._scanTo(',') != -1:
        #         skipped = self._skipTo(',')
        #     else:
        #         skipped = self._skipTo('}')
        #         done = True
        #     if skipped == None:
        #         return False
        #     self.field_value = skipped[:-1].strip()
        #     self._submit()
        #     self.state = '{'
        #     return done
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
        field_name = self.field_name
        if self.parent_field_name != None:
            field_name = self.parent_field_name + "." + field_name
        self.callback(field_name, self.field_value)

    def _skipWS(self):
        bufLen = len(self.buffer)
        for i in range(bufLen):
            c = self.buffer[i]
            if not c.isspace():
                self.buffer = self.buffer[i:]
                return True
        self.buffer = ""
        return False

    def _skipTo(self, what):
        return self.__skip(what, True)

    # def _skipUntil(self, what):
    #     return self.__skip(what, False)

    def __skip(self, what, inclusive):
        i = self._scanTo(what)
        if i == -1:
            self.skipping += self.buffer
            self.buffer = ""
            return None
        if inclusive:
            skipped = self.skipping + self.buffer[0:i+1]
            self.buffer = self.buffer[i+1:]
        else:
            skipped = self.skipping + self.buffer[0:i]
            self.buffer = self.buffer[i:]
        self.skipping = ""
        return skipped

    def _scanTo(self, what):
        bufLen = len(self.buffer)
        for i in range(bufLen):
            c = self.buffer[i]
            if c == what:
                return i
        return -1



import random


json = '{"NESTED":{"str":"str value","int":123},"INT":4321,"NESTED2":{"str2":"str value2"},"STR":"STR VALUE"}'
#json = ' { "int" : 123 , "int2" : 999 , "str" : "str value" , "str2" : "str value 2" , "Ftrue" : true, "Ffalse" : false , "Fnull" : null, "end":"END" }'

print("***")
parser = JsonStreamParser(lambda id, val: print(". TEST1 -- " + "'" + id + "':'" + val + "'"))
parser.sinkJsonData(json)

print("***")
parser = JsonStreamParser(lambda id, val: print(". TEST2 -- " + "'" + id + "':'" + val + "'"))
json_data = ""
for c in json:
    json_data += c
    if random.random() > 0.5:
        parser.sinkJsonData(json_data)
        json_data = ""
if json_data != "":
    parser.sinkJsonData(json_data)
