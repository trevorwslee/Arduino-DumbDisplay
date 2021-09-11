


class JsonStreamParser:
    '''
    :param callback
      input: element id, element valud
    '''
    def __init__(self, callback):
        self.state = None
        self.buffer = ""
        self.skipping = ""
        self.field_name = None
        self.field_value = None
        self.callback = callback

    def sinkJsonData(self, data):
        self.buffer = self.buffer + data
        while True:
            if self.buffer == '':
                break
            self._streamParse()

    def _streamParse(self):
        if self.state == None:
            if self._skipTo('{') == None:
                return
            self.state = '{'
        if self.state == '{':
            if self._skipTo('"') == None:
                return
            self.state = '{>'
        if self.state == '{>':
            skipped = self._skipTo('"')
            if skipped == None:
                return
            self.field_name = skipped[:-1].strip()
            self.state = '>:'
        if self.state == '>:':
            if self._skipTo(':') == None:
                return
            self.state = ':'
        if self.state == ':':
            if not self._skipWS():
                return
            self.state = '^'
        if self.state == '^':
            c = self.buffer[0]
            if c == '"':
                self.buffer = self.buffer[1:]
                self.state = '^>"'
            else:
                self.state = '^>'
        if self.state == '^>"':
            skipped = self._skipTo('"')
            if skipped == None:
                return
            self.field_value = skipped[:-1].strip()
            self._submit()
            self.state = '{'
        if self.state == '^>':
            if self._scanTo(',') != -1:
                skipped = self._skipTo(',')
            else:
                skipped = self._skipTo('}')
            if skipped == None:
                return
            self.field_value = skipped[:-1].strip()
            self._submit()
            self.state = '{'

    def _submit(self):
        self.callback(self.field_name, self.field_value)

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



parser = JsonStreamParser(lambda id, val: print("'" + id + "':'" + val + "'"))

parser.sinkJsonData(' { "int" : 123 , "int2" : 999 , "str" : "str value" , "Ftrue" : true, "Ffalse" : false , "Fnull" : null } ')
