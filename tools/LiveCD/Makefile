
LANG_TEMP_DIR=Data/Language/.Temp
all: language

language:
	[ -e "$(LANG_TEMP_DIR)" ] || mkdir $(LANG_TEMP_DIR)
	for file in ConfigureLiveCD KeymapDialog; \
	do cat bin/$$file | sed "s/tr \(.*\)/tr\(\1\)/g" > $(LANG_TEMP_DIR)/$$file; \
	done
	
	cd $(LANG_TEMP_DIR)/../; pylupdate LiveCD.pro

	rm -f $(LANG_TEMP_DIR)/*
	rmdir $(LANG_TEMP_DIR)
