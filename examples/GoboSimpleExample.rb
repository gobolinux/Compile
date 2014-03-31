#!/usr/bin/ruby

# Workaround lack of ruby Environment
$: << "/System/Index/lib/ruby/site_ruby/1.8"

require 'gobo'


### App Info ##################################################################
#
#  Create a GoboApplication object and set
app = GoboApplication.new()

app.description = "Simple example showing GoboLinux's Ruby Framework."
app.credits     = "Released under the GNU GPL."
app.usage       = "<anything>"
app.example     = "foo bar baz"
app.notes = <<END_OF_NOTES
The options don't really do anything.

Default behavior for --same is 'cancel', for --old is 'keep'.
Notice that 'remove' may be dangerous for important system packages
as it can leave the system in an inconsistent state during installation.
END_OF_NOTES

app.helpOnNoArguments = true

# add options to be handled.  The default help screen is generated from this
# information
#
# app.addOptionBoolean(nameString, aliasesArray, descriptionString)
# app.addOptionEntry(nameString, aliasesArray, descriptionString, defaultString)
# app.addOptionList(nameString, aliasesArray, descriptionString, defaultString)
#
# The values are accessible through app.options[nameString]

app.addOptionBoolean("batch", ["-b"], "Do not ask for confirmation")
app.addOptionBoolean("debug", ["-d"], "Step-by-step execution on source packages.
        (Has no effect on binary packages.)")
app.addOptionEntry("same", ["-s"], "What to do when unpackaging over the same version,
        'keep', 'remove', 'ask' or 'cancel'.", "ask")
app.addOptionEntry("old", ["-o"], "What to do with a previously existing version of a package if found,
        'keep', 'remove', 'ask' or 'cancel'.", "keep")


### Operation #################################################################
# Re-Define app.run    This is the main entry point.  
#

def app.run
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
end

# This starts the GoboApplication.  Options are processed
app.start

