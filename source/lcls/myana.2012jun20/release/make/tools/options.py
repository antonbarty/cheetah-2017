import sys
import getopt

class Options(object):
  def __init__(self, mandatory, optional=[], switches=[]):
    self.opts = {}

    self.__mand = mandatory
    self.__optn = optional
    self.__swtc = switches

  def __getattr__(self, attribute):
    if attribute in self.opts:
      return self.opts[attribute]
    return None

  def usage(self, notes=''):
    msg = 'usage: %s' %(sys.argv[0])
    for option in self.__mand:
      msg += ' --%s <%sname>' %(option, option)
    for option in self.__optn:
      msg += ' [--%s <%sname>]' %(option, option)
    for option in self.__swtc:
      msg += ' [--%s]' %(option)
    if notes != '':
      msg += '\nnotes: '
      msg += notes
    print msg


  def parse(self):
    opts = []
    for option in self.__mand:
      opts.append(option+'=')
    for option in self.__optn:
      opts.append(option+'=')
    for option in self.__swtc:
      opts.append(option)
    results = getopt.getopt(sys.argv[1:], '', opts)
    if len(results[1]) > 0:
      raise RuntimeError, 'unknown argument(s) \'%s\'' %(results[1])
    for opt in results[0]:
      self.opts[opt[0][2:]] = opt[1]
    for option in self.__mand:
      if option not in self.opts:
        raise RuntimeError, 'mandatory option \'--%s\' not found' %(option)

