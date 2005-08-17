
DIR=../lib/python2.3/site-packages/

all: $(DIR)/ManagerUI.py $(DIR)/ManagerConfigForm.py $(DIR)/LoggersWrapperForm.py


$(DIR)/LoggersWrapperForm.py: LoggersWrapperForm.ui
	pyuic LoggersWrapperForm.ui > $(DIR)/LoggersWrapperForm.py

$(DIR)/ManagerUI.py: ManagerUI.ui
	pyuic ManagerUI.ui > $(DIR)/ManagerUI.py

$(DIR)/ManagerConfigForm.py: ManagerConfigForm.ui
	pyuic ManagerConfigForm.ui > $(DIR)/ManagerConfigForm.py
	

$(DIR)/GraphicalTailForm.py: graphicaltailform.ui
	pyuic graphicaltailform.ui > $(DIR)/GraphicalTailForm.py

$(DIR)/GraphicalMultiTailForm.py: GraphicalMultiTailForm.ui
	pyuic GraphicalMultiTailForm.ui > $(DIR)/GraphicalMultiTailForm.py

