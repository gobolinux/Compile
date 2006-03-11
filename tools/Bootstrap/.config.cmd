deps_config := \
	packages/Shared-Mime-INFO/Config.in \
	packages/LibXML2/Config.in \
	packages/LibART/Config.in \
	packages/Startup-Notification/Config.in \
	packages/LibOIL/Config.in \
	packages/Expat/Config.in \
	packages/Gettext/Config.in \
	packages/Mowitz/Config.in \
	packages/Dbus/Config.in \
	packages/GTK+/Config.in \
	packages/Pango/Config.in \
	packages/ATK/Config.in \
	packages/GLib/Config.in \
	packages/FreeType/Config.in \
	packages/Fontconfig/Config.in \
	packages/LibVorbis/Config.in \
	packages/LibOGG/Config.in \
	packages/LibMad/Config.in \
	packages/SDL/Config.in \
	packages/TIFF/Config.in \
	packages/JPEG/Config.in \
	packages/LibPNG/Config.in \
	packages/Swfdec/Config.in \
	packages/Siag/Config.in \
	packages/RXVT/Config.in \
	packages/ROX-Filer/Config.in \
	packages/Matchbox/Config.in \
	packages/Nano-X/Config.in \
	packages/Xorg/Config.in \
	packages/OpenSSH/Config.in \
	packages/OpenSSL/Config.in \
	packages/Links/Config.in \
	packages/Strace/Config.in \
	packages/GCC/Config.in \
	packages/GMP/Config.in \
	packages/Bash/Config.in \
	packages/Ncurses/Config.in \
	packages/Udev/Config.in \
	packages/Listener/Config.in \
	packages/Hotplug/Config.in \
	packages/GoboHide/Config.in \
	packages/GPM/Config.in \
	packages/ZLib/Config.in \
	packages/LibGCC/Config.in \
	packages/BusyBox/Config.in \
	packages/UClibc/Config.in \
	packages/Glibc/Config.in \
	Config.in

.config include/config.h: $(deps_config)

$(deps_config):
