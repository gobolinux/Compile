#!/usr/bin/env ruby
# This code parses hints files, which are expected to be in a syntax like this
# sample:
#     Settings/id_*: keep_existing, compare_exist
#     Settings/sshd_config, Settings/whatever: auto_merge, *
#     *: create
#
# The general form of this is expressed in EBNF here:
#    <hints_file> ::= '[' <keyword> ']' <hint_list> 
#                    | '[' <keyword> ']' <hint_list> <hints_file>
#    <hint_list> ::= <hint> | <hint> <hint_list>
#    <hint> ::= <file_list> ':' <keyword_list>
#    <file_list> ::= <file> | <file>, <file_list>
#    <file> ::= (any valid uinx file specification, including wildcards)
#    <keyword_list> ::= <keyword> | <keyword>, <keyword_list>
#    <keyword> ::= [A-Za-z_]+
#
# I considered using Bison and Flex for this, but that seemed like it would be
# excessive overkill. Instead, this is a simple recursive descent kind of thing
#
# Author:: Dan Charney
# Date::   10 June 2006

# Before I do anything else, I'll improve Strings a bit.
class String
   # Why Ruby doesn't just include this function is beyond me
   def begins_with(regexOrString)
      return nil unless to_s.index(regexOrString) === 0
      to_s[regexOrString]
   end

   def extract(regexOrString)
      return nil unless size > 0
      r = begins_with(regexOrString)
      return nil unless r
      replace self[r.size, size]
      return r
   end
end


# and now, your basic simple recursive descent parser:
def hint_list(src)
   src.strip!
   $cout << "   case $filename in\n" 
   while src.size > 0 && hint(src)
   end
   $cout << "\n   esac # esac is ridiculous\n"
end

def hint(src)
   bak = $cout
   unless file_list(src) && src.extract(/\s*:\s*/) && keyword_list(src)
      $cout = bak
      src = "<End of File>" unless src.size > 0
      src = src[0, 60].gsub('"', '\"')
      $cerr << "Log_Error 'Parse error in hints file near \\\"#{src}\\\"'"
      return false
   end
   return true
end

def file_list(src)
   $cout << "\n      "
   return false unless file(src)
   while src.extract(/\s*,\s*/)
      $cout << '|'
      return false unless file(src)
   end
   $cout << ') '
   return true
end

def keyword_list(src)
   $cout << "hints=(\n         "
   return false unless keyword(src)
   while src.extract(/\s*,\s*/)
      $cout << "\n         "
      return false unless keyword(src)
   end
   $cout << "\n         ) ;;\n      "
   return true
end

def file(src)
   r =  src.extract(/\s*[^\s,:]+\s*/) 
   $cout << r.strip if r
   return r
end

def keyword(src)
   r = src.extract(/\s*[^\s,:]+\s*/)
   $cout << bashsafe(r.strip) if r
   return r
end

def bashsafe(src)
   return src.gsub(/\s/, '_').gsub(/[^\w_]/, '')
end



# Read the input file, strip unnecessary spaces
source = File.new(ARGV[0]).read.gsub(/#[\s\S]*?[\r\n]/, "\r").
   gsub(/[\s]+/, ' ').strip

$cout = ""
$cerr = "" 

$cout << <<EOF
function Hints_For() {
   local filename="$1"
   local hints=''
EOF

source.split('[').each do | section |
   section.extract(/\s*/)
   next unless section.extract(ARGV[1])
   section.extract(/[\s\]]*/)
   hint_list(section)
end


$cout << "\n   echo \"${hints[@]}\""
$cout << "\n}"

print $cout unless $cerr.size > 0
print $cerr
