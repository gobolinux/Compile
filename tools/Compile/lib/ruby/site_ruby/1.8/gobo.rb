### Changelog #################################################################

# 30/08/2003 - Initial Version

=begin

gobo.rb - GoboLinux Ruby Module

=end
require 'getoptlong'
require 'gobo/logconsole.rb'
require "open3"
#require 'process'

def Dir.empty?(dir)
  begin
    return Dir.entries(dir).length == 2
  rescue Errno::ENOENT
    return true
  end
end

module Gobo
  DEFAULT_ARCH = "i686"
  def Gobo.RealPath (symLink)
    return `readlink -f #{symLink}`.chomp!
  end

  def Gobo.goodLink?(symlink)
    return false unless FileTest.symlink?(symlink)
    return FileTest.exists?(File.readlink(symlink))
  end

  def Gobo.brokenLink?(symlink)
    return false unless FileTest.symlink?(symlink)
    return ! FileTest.exists?(File.readlink(symlink))
  end

  def Gobo.smartNumber(number)
    string = number.to_s
    return string unless string.include? "."
    string[0, string.index('.') + 3]
  end

  def Gobo.smartSize(size)
    return size.to_s + " bytes" if size > 0 and size < 1023.99
    return "#{smartNumber(size / 1024.to_f)} Kb (#{smartNumber(size)} bytes)" if size >= 1024 and size < 1048576
    return "#{smartNumber(size / 1048576.to_f)} Mb (#{smartNumber(size)} bytes)" if size >= 1048576 and size < 1073741824
    return "#{smartNumber(size / 1073741824.to_f)} Gb (#{smartNumber(size)} bytes)" if size >= 1073741824 and size < 1099511627776
    return "#{smartNumber(size / 1099511627776.to_f)} Tb (#{smartNumber(size)} bytes)" if size >= 1099511627776 and size < 1125899906842624
    return "#{smartNumber(size / 1125899906842624.to_f)} Pb (#{smartNumber(size)} bytes)" if size >= 1125899906842624
  end

  def Gobo.getUsername
    name = ENV['USER']
    if Process.uid == 0 && (name == "gobo" || name == "root")
      if ENV["SUDO_USER"]
        name = ENV["SUDO_USER"]
      else
        puts "You are currently logged in as #{ENV['USER']}."
        puts "Please enter your regular user name so this devel version"
        $stdout.print "can be properly tagged: "
        $stdout.flush
        name = $stdin.gets
      end
    end
    return name
  end

  def Gobo.chdir(dir, &block)
    Dir.chdir(dir) {
      block.call
    }
#     begin
#     pwd = Dir.getwd
#     Dir.chdir(dir)
#     block.call(pwd)
#     rescue
#       raise
#     ensure
#       Dir.chdir(pwd)
#     end
  end # End of Gobo.chdir

  def Gobo.smartPOpen(cmd, txt, block = nil)
    $stdout << "exec:" << cmd << "\n"
    IO.popen(cmd, "r") { |io|
      io.each { |msg|
        block.call("STDOUT:#{txt}#{msg}")
      }
    }
#     stdin, stdout, stderr = Open3.popen3(cmd)
#     readers = Array[stdout, stderr, $stdin]
#     $stdout << readers.inspect << "\n"
#     $stdout << readers.inspect << "\n"
# #    (read, write, error) = select(readers,10)
#     select(readers,10)
  end # End of Gobo.smartPOpen

  def Gobo.parseConfig(filename)
    conf = Hash.new
    File.open(filename) { |file|
      data = file.read().chomp!
      data.gsub!(/\n(\s+)/, '\1')
      data.split(/\n/).each { |entry|
        (name, value) = entry.split(/: /)
        conf[name] = value
      }
    }
    return conf
  end # End of Gobo.parseConfig
end # End of Module Gobo

module GoboPath
  rubyblock = `GoboPath2Ruby`
  eval rubyblock
end

class GoboApplication
  include GoboLogConsole
  public
    attr_accessor :description, :credits, :usage, :example
    attr_accessor :notes, :helpOnNoArguments
    attr_reader   :name, :version, :startDir

  public
    def initialize()
      # Add default switches
      @options = Hash.new
      addOptionBoolean("help", ["-h"], "Display this help")
      addOptionBoolean("version", ["-v"], "Show program version")
      addOptionBoolean("verbose", ["-V"], "Enable verbose mode")
      addOptionBoolean("debug", [], "Enable debug mode")

      @name = File.basename($0)
      @version = File.basename(File.dirname(Gobo.RealPath(Gobo.RealPath("#{$0}/.."))))
      @startDir = Dir.getwd
      $startDir = @startDir
      setLogLevel(LogLevelNormal)
#      setLogLevel(LogLevelDebug)
    end

    def start()
      # Save argument list
      @savedArgs = ARGV.clone
      # Fill getopts with array of arrays for GetoptLong
      getopts = Array.new
      @options.each do |key, value|
        if value["type"] == "entry"
          if value["value"] == nil
	         flag = GetoptLong::REQUIRED_ARGUMENT
	      else
	         flag = GetoptLong::OPTIONAL_ARGUMENT
          end
        elsif value["type"] == "list"
           flag = GetoptLong::OPTIONAL_ARGUMENT
        else
	       flag = GetoptLong::NO_ARGUMENT
        end
        getopts.push( ["--#{key}",  value["switches"], flag].flatten)
      end

      # Process options
      begin
        opts = GetoptLong.new(*getopts)
        opts.quiet=true
        opts.each { |opt, arg|
          value = @options[opt[2..-1]]
          case value["type"]
            when "boolean"
              value["value"] = true
            when "entry"
              value["value"] = arg if arg.length > 0
            when "list"
              value["value"] = arg.split(":")
          end
        }
      rescue GetoptLong::InvalidOption =>msg
        logVerbose "Invalid option: #{msg}"
      end

      help if options("help")
      help if ARGV.size == 0 && helpOnNoArguments == true
      version if options("version")
      setLogLevel(LogLevelVerbose) if options("verbose")
      setLogLevel(LogLevelDebug) if options("debug")
      run()
    end

    def stop(msg = nil)
      logError "#{msg}" if msg
      exit(0)
    end

    def run()
    end

    def options(key)
	@options[key]["value"]
    end

    def setOption (name, value = true)
	@options[name]["value"] = value
    end

    def addOption (type, name, switches, description, default)
      # Make sure switches is an array
      switches = [switches] unless switches.is_a?(Array)
      @options[name] = {"switches" => switches, "type" => type,
               "description" => description, "value" => default}
    end
    def addOptionBoolean (name, switches, description)
      addOption("boolean", name, switches, description, false)
    end

    def addOptionEntry (name, switches, description, default = nil)
      addOption("entry", name, switches, description, default)
    end

    def addOptionList (name, switches, description, default = nil)
      addOption("list", name, switches, description, default)
    end

    def version()
      logNormal "#{@name} #{@version}"
      logNormal ""
      logNormal @credits if @credits
      stop()
    end

    def help()
      logNormal @description if @description
      logNormal ""
      logNormal "Usage: #{@name} #{@usage}"
      logNormal
      logNormal "Options:"
      @options.sort.each do |x|
        (key, value) = x
        case value["type"]
          when "boolean"
            logNormal "     --#{key}, " << value["switches"].join(", ")
            logNormal "        #{value["description"]}"
          when "entry"
            logNormal "     --#{key}, " << value["switches"].join(", ") << "<entry>"
            logNormal "        #{value["description"]}"
            logNormal "        The default value is #{value["value"]}" if value["value"]
          when "list"
            logNormal "     --#{key}, " << value["switches"].join(", ") << "<entry>[:<entry>...]"
            logNormal "        #{value["description"]}"
            logNormal "        The default value is #{value["value"]}" if value["value"]
        end
      end
      logNormal "\n\nNotes:\n    #{@notes}\n" if @notes
      logNormal "Examples:\n    #{@name} #{@example}" if @example
      stop()
    end

    def verifySuperuser
      if ENV['ROOTLESS_GOBOLINUX']
        logNormal "Bypassing verification for superuser"
      else
        if Process.uid != 0
          logVerbose "Running as superuser..."
          # Revalidate password for another 5 mins"
          system 'sudo -u "#0" -v' 
          # Run with superuser's HOME.
          exec "sudo " << ["-u", "\"#0\"", "-H", $0, "#{@savedArgs.join(' ')}"].join(" ")
        end
      end
    end # End of verifySuperuser
 
end # End of GoboApplication

class GoboCallbacks < Hash
  public
    def initialize()
      super()
    end

    def addCallback(callName, aProc = nil)
      if (aProc != nil)
        if aProc.respond_to? :call
          self[callName] = aProc
        else
          raise "#{callName} callback has no call method"
        end
      else
        self[callName] = proc { |args|
          yield *args
        }
      end
    end

    def call(callName, *args)
      self[callName].call(*args) if self[callName]
    end

  protected
    def []=(callName, aProc)
      if callName && (aProc.respond_to? :call)
         super
      else
        raise "#{callName} callback has no call method"
      end
    end
  private
    def checkSelf
      self.each { |callName, aProc|
        raise "#{callName} callback has no call method" if aProc.respond_to? :call
      }
    end
end
