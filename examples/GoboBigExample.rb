#!/usr/bin/ruby

# Workaround lack of ruby Environment
$: << "/System/Index/lib/ruby/site_ruby/1.8"

require 'gobo'


### App Info ##################################################################
# Subclass GoboApplication
class GoboBigExample < GoboApplication
  public
    def initialize()
      super()
    
      self.description = "More complex example showing GoboLinux's Ruby Framework.."
      self.credits     = "Released under the GNU GPL."
      self.usage       = "<anything>"
      self.example     = "foo bar baz"
      self.helpOnNoArguments = true

      self.notes = <<END_OF_NOTES
The options don't really do anything.

Default behavior for --same is 'cancel', for --old is 'keep'.
Notice that 'remove' may be dangerous for important system packages
as it can leave the system in an inconsistent state during installation.
END_OF_NOTES


# add options to be handled.  The default help screen is generated from this
# information
#
# app.addOptionBoolean(nameString, aliasesArray, descriptionString)
# app.addOptionEntry(nameString, aliasesArray, descriptionString, defaultString)
# app.addOptionList(nameString, aliasesArray, descriptionString, defaultString)
#
# The values are accessible through app.options[nameString]

     addOptionBoolean("batch", ["-b"], "Do not ask for confirmation")
     addOptionBoolean("debug", ["-d"], "Step-by-step execution on source packages.
        (Has no effect on binary packages.)")
     addOptionEntry("same", ["-s"], "What to do when unpackaging over the same version,
        'keep', 'remove', 'ask' or 'cancel'.", "ask")
     addOptionEntry("old", ["-o"], "What to do with a previously existing version of a package if found,
        'keep', 'remove', 'ask' or 'cancel'.", "keep")


    end

    def run
      $stdout << "Handled args\n"
      puts "Batch: " << options("batch").to_s
      puts "Debug: " << options("debug").to_s
      puts "Same: " << options("same").to_s
      puts "Old: " << options("old").to_s

      puts
      $stdout << "Unhandled args\n"
      ARGV.each do |arg|
        $stdout << arg << "\n"
      end

      $stdout << "In run loop" << "\n" 
    end # End of run()
end  # End of GoboBigExample class

app = GoboBigExample.new()
app.start


