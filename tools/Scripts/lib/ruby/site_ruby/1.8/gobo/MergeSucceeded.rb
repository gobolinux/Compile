#!/usr/bin/env ruby
# This program tries to figure out whether two files auto-merged correctly by
# checking for duplicate lines in configuration files.

def die
   exit 1
end

if ARGV.size < 5 then
   print "Usage: MergeSucceeded.rb <old_file> <user_file> <default_file> \\\n"\
      << "                            <new_file> <fudge_factor>\n"
   die
end

$fudge = ARGV[4].to_i

class FileProfile
   def initialize(filename = nil, subtract = nil)
      @fullLines = {}
      @beforeFirstSpace = {}
      @beforeSymbol = {}
      if subtract then
         subtract.fullLines.each_pair        { |k,v| @fullLines[k]        = -v }
         subtract.beforeFirstSpace.each_pair { |k,v| @beforeFirstSpace[k] = -v }
         subtract.beforeSymbol.each_pair     { |k,v| @beforeSymbol[k]     = -v }
      end
      if filename then
         File.open(filename).readlines.each do |line|
            handleLine(line)
         end
      end
   end

   attr_reader :fullLines, :beforeFirstSpace, :beforeSymbol

   def handleLine(line, factor = 1)
      line.strip!
      bfs = line[/\S+/]
      bs  = line[/[\[\w\s]+/]
      @fullLines[line]       = (@fullLines[line]       || 0) + factor
      @beforeFirstSpace[bfs] = (@beforeFirstSpace[bfs] || 0) + factor if bfs
      @beforeSymbol[bs]      = (@beforeSymbol[bs]      || 0) + factor if bs
   end
end

o = FileProfile.new(ARGV[0])
u = FileProfile.new(ARGV[1], o)
d = FileProfile.new(ARGV[2], o)
n = FileProfile.new(ARGV[3], o)

for x in ['fullLines', 'beforeFirstSpace', 'beforeSymbol']
   keys = (eval "u.#{x}.keys + d.#{x}.keys + n.#{x}.keys").uniq
   keys.each do |key|
      for y in ['u', 'd', 'n']
         eval("#{y}.#{x}[key] ||= 0")
      end
      arr = eval("[u.#{x}[key], d.#{x}[key]]").sort
      nv = eval("n.#{x}[key]")
      $fudge -= [(arr[0] - nv), 0].max
      $fudge -= [(nv - arr[1]), 0].max
      die if $fudge < 0
   end
end

exit 0
