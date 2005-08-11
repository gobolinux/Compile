#!/usr/bin/perl

# (C) 2005 Carlo J. Calica. Relesed under the GNU GPL.

# Usage: (note change usage() as well)
#    mkinitrd.pl <root_dev> <initrd_dir> <modules_base> <additional_modules>
# Example:
#    mkinitrd.pl /Mount/loop /System/Kernel/Modules/2.6.8.1-Gobo foo /path/to/bar.ko

### Changelog #################################################################

# 2005/03/04 - [calica] add usage output on 0 args
# 2005/03/02 - [calica] Initial version

# Globally scope vars
my $root_dev;
my $initrd_dir;
my $mod_base;
my %avail_modules;
my @wanted_modules;
my @add_mod_files;

# Main entry point.  Called at end.
sub main {
   usage() if $#ARGV == 0;
   $root_dev = shift(@ARGV);
   $initrd_dir = shift(@ARGV);
   $mod_base = shift(@ARGV);
   
   # Load the dirs of modules we care about
   load_avail_modules("$mod_base/kernel/drivers/scsi");
   load_avail_modules("$mod_base/kernel/drivers/usb");
   
   # Load list of desired modules   
   load_wanted_modules();
   process_modules();
   
   # Create initrd and linuxrc
   create_initrd();
   create_linuxrc();
}

sub usage {
   print <<END_OF_MSG;
Usage:
    mkinitrd.pl <root_dev> <initrd_dir> <modules_base> <additional_modules>
Example:
    mkinitrd.pl /Mount/loop /System/Kernel/Modules/2.6.8.1-Gobo foo /path/to/bar.ko
END_OF_MSG
   exit
}
   
# Create dictionary of module name/location pairs
sub load_avail_modules {
   my $dir = shift;   
   my $fh, $cmd, $module;
   
   $cmd = "find $dir -type f|";
   open($fh, "$cmd") || die "Couldn't exec $cmd";
   while(<$fh>) {
      chomp;
      @parts = split("/");
      $module = $parts[$#parts];
      $module =~ s/\.ko//;
      $avail_modules{$module}=$_;
   }
   
}

# Get list of want modules from "lsmod" and ARGV
sub load_wanted_modules {
   my $fh, $cmd, $module, $item;
   
   # Get output from lsmod
   $cmd = "lsmod|";
   open($fh, "$cmd") || die "Couldn't exec $cmd";
   while(<$fh>) {
      chomp;
      ($module) = split(/\s+/);
      next if $module eq "Module";
      unshift(@wanted_modules, $module);
   }

   # Handle the modules from the args
   foreach $item (@ARGV) {
      if ( -f $item ) { # its a path to a kernel module
         push(@add_mod_files, $item);
      }
      else {      
         unshift(@wanted_modules, $item); # Change this to add on top 
#         push(@wanted_modules, $item);   # or bottom
      }
   }
}


# Foreach wanted module, find the file and append to @add_mod_files
sub process_modules {
   my $module;
   foreach $module (@wanted_modules) {
      $file = $avail_modules{$module};
      next if $file eq "";
      push(@add_mod_files, $avail_modules{$module});
   }
}

# Copy the specified program to initrd and symlink
sub copy_gobo_programs {
   local $prog_name = shift;
   local $src_dir, $prog_version;
    
   $src_dir = `readlink -f /Programs/$prog_name/Current/`;
   chomp($src_dir);
   $prog_version = `basename $src_dir`;
   chomp($prog_version);
   
   mkdir("Programs/$prog_name");
   system("cp -Rp $src_dir Programs/$prog_name");
   (-d "$src_dir/../Settings") && system("cp -Rp /Programs/$prog_name/Settings Programs/$prog_name");
   symlink("$prog_version", "Programs/$prog_name/Current");
   system("RescueSymlinkProgram $src_dir System/Links 2>/dev/null");
}

# Create the initrd filesystem and copy packages.
sub create_initrd() {

   chdir($initrd_dir);
   system("rm -rf *");
   
   # Create base dir structure
   mkdir("Programs");
   mkdir("System");
   mkdir("System/Links");
   mkdir("System/Links/Executables");
   mkdir("System/Links/Libraries");
   mkdir("System/Settings");
   mkdir("System/Kernel");
   mkdir("System/Kernel/Devices");
   mkdir("System/Kernel/Status");
   mkdir("System/Kernel/Objects");
   symlink("System/Links/Executables", "bin");
   symlink("System/Links/Executables", "sbin");
   symlink("System/Links/Libraries", "lib");
   symlink("System/Settings", "etc");
   symlink("System/Kernel/Devices", "dev");
   symlink("System/Kernel/Status", "proc");
   symlink("System/Kernel/Objects", "sys");
   mkdir("Programs");
   mkdir("new-root");
   
   # Install packages
   copy_gobo_programs("Busybox");
   copy_gobo_programs("Udev");
   
   system("cp /Programs/Scripts/Current/Data/initrd/Udev System/Links/Executables/Udev-start");
   system("touch etc/busybox.conf");
}

# Create the linuxrc and copy modules
sub create_linuxrc() {
   
   my $insmod_strs;
   # Add modules to initrd
   foreach $file (@add_mod_files) {
      system("cp $file lib");
      @parts = split("/", $file);
      $module = $parts[$#parts];
      
      $insmod_strs .= "insmod /lib/$module\n";
   }
   
   # Create linuxrc
   open($fh, ">linuxrc") || die "Unable to write linuxrc\n";
   print $fh <<END_OF_SECTION;
#!/bin/ash

export PATH=/bin:/sbin

# Start udev
Udev-start

# Load modules
$insmod_strs

# Mount root filesystem
mount -o ro $root_dev /new-root
cd /new-root
pivot_root . initrd
#ash
exec chroot . sbin/init <dev/console >dev/console 2>&1

END_OF_SECTION
   close($fh);
   system("chmod a+x linuxrc");
}   

# Call main() function at top of page.
main();

