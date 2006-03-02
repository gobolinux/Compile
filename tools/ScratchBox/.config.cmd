deps_config := \
	package/Shared-Mime-INFO/Config.in \
	package/LibXML2/Config.in \
	package/LibART/Config.in \
	package/Startup-Notification/Config.in \
	package/LibOIL/Config.in \
	package/Expat/Config.in \
	package/Gettext/Config.in \
	package/Mowitz/Config.in \
	package/Dbus/Config.in \
	package/GTK+/Config.in \
	package/Pango/Config.in \
	package/ATK/Config.in \
	package/GLib/Config.in \
	package/FreeType/Config.in \
	package/Fontconfig/Config.in \
	package/LibVorbis/Config.in \
	package/LibOGG/Config.in \
	package/LibMad/Config.in \
	package/SDL/Config.in \
	package/TIFF/Config.in \
	package/JPEG/Config.in \
	package/LibPNG/Config.in \
	package/Xorg-App/Config.in \
	package/Swfdec/Config.in \
	package/Siag/Config.in \
	package/RXVT/Config.in \
	package/ROX-Filer/Config.in \
	package/Matchbox/Config.in \
	package/Nano-X/Config.in \
	package/Xorg/Config.in \
	package/Xorg-Lib/Config.in \
	package/OpenSSH/Config.in \
	package/OpenSSL/Config.in \
	package/Links/Config.in \
	package/Strace/Config.in \
	package/GCC/Config.in \
	package/GMP/Config.in \
	package/Bash/Config.in \
	package/Ncurses/Config.in \
	package/Udev/Config.in \
	package/Listener/Config.in \
	package/Hotplug/Config.in \
	package/GoboHide/Config.in \
	package/GPM/Config.in \
	package/ZLib/Config.in \
	package/LibGCC/Config.in \
	package/BusyBox/Config.in \
	package/UClibc/Config.in \
	package/Glibc/Config.in \
	Config.in

.config include/config.h: $(deps_config)

$(deps_config):
