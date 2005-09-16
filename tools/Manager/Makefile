
 # $@ is the name of the file to be made. 
 # $? is the names of the changed dependents.
 # $< the name of the related file that caused the action. 
 # $* the prefix shared by target and dependent files.

GUI_DIR=GUI
LIB_DIR=lib/python2.3/site-packages
BIN_DIR=bin

all: guis libs

guis : $(LIB_DIR)/ManagerUI.py $(LIB_DIR)/ManagerConfigForm.py $(LIB_DIR)/LoggersWrapperForm.py $(LIB_DIR)/GenericEditor.py

$(LIB_DIR)/LoggersWrapperForm.py: $(GUI_DIR)/LoggersWrapperForm.ui
	pyuic $? > $@

$(LIB_DIR)/GenericEditor.py: $(GUI_DIR)/GenericEditor.ui
	pyuic $? >$@

$(LIB_DIR)/ManagerUI.py: $(GUI_DIR)/ManagerUI.ui
	pyuic $? > $@

$(LIB_DIR)/ManagerConfigForm.py: $(GUI_DIR)/ManagerConfigForm.ui
	pyuic $? > $@


libs : $(LIB_DIR)/GetInstalled.py $(LIB_DIR)/ManagerRunner.py compilation

$(LIB_DIR)/GetInstalled.py : $(BIN_DIR)/GetInstalled
	cd $(LIB_DIR) && ln -sfn ../../bin/GetInstalled ./GetInstalled.py

$(LIB_DIR)/ManagerRunner.py : $(BIN_DIR)/ManagerRunner
	cd $(LIB_DIR) && ln -sfn ../../bin/ManagerRunner ./ManagerRunner.py
	
compilation :
	DirPythonCompile $(LIB_DIR)

clean :
	rm -f $(LIB_DIR)/*.pyo $(LIB_DIR)/*.pyc $(LIB_DIR)/*.py
