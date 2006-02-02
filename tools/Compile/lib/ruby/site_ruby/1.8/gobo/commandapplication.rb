=begin

gobo/commandapplication.rb - GoboLinux CommandApplication Ruby Module

Mostly stubs

=end
require 'gobo'

class GoboCommandApplication < GoboApplication
  public        # Public attributes
  private       # Private instance/class variables
  public        # Public methods
    def initialize()
      super()
      @commands = GoboCallbacks.new
      @descriptions = Hash.new
    end

    def addCommand(cmdName, description, aProc = nil)
      aProc = proc {|args|
        yield *args
      } if aProc == nil
      @commands.addCallback(cmdName, aProc)
      @descriptions[cmdName] = cmdDescription
    end

    def run()
      super()
      command = ARGV.shift
      if @commands[command]
        @commands[command].call()
      else
        puts "#{command} not a valid command"
      end
    end
  private       # Private methods
end
