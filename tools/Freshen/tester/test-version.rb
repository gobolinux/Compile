#!/usr/bin/ruby
# Copyright (C) 2007 Michael Homer
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
require 'lib/version.rb'
require 'tester/tester.rb'

class VersionTester < Tester
	def test(a,b,test="<=>")
		a = Version.new(a)
		b = Version.new(b)
		case test
		when '<=>'
			assert("#{a} < #{b}", a<b)
			assert("!(#{a} > #{b})", !(a>b))
			assert("#{b} > #{a}", b>a)
			assert("!(#{b} < #{a})", !(b<a))
			assert("!(#{a} == #{b})", !(a==b))
		when '=='
			assert("!(#{a} < #{b})", !(a<b))
			assert("!(#{a} > #{b})", !(a>b))
			assert("!(#{b} > #{a})", !(b>a))
			assert("!(#{b} < #{a})", !(b<a))
			assert("#{a} == #{b}", a==b)		
		end
	end
end

t = VersionTester.new

unequals = [
'0.9.8',
'1.0pre8',
'1.0rc1',
'1.0rc2',
'1.0-r1',
'1.0-r2',
'1.0a-r2',
'1.0r-r2',
'1.0.1',
'1.1',
'1.10',
'2.0.0',
'2.0.0.5',
'2.12-r1',
'2.12q',
'2.12r',
'2.12r-r3'
]

0.upto(unequals.length-1) {|i|
	(i+1).upto(unequals.length-1) {|j|
		t.test(unequals[i],unequals[j])
	}
}
t.test('4.0.0','4.0', '==')