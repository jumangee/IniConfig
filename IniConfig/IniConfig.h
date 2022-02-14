#pragma once

#include <Arduino.h>
#include <FS.h>

#define DEBUGLN(t) Serial.println(t);
#define DEBUG(t) Serial.print(t);

class IniConfig;

class IniConfigItem {
	public:
		String name;

        IniConfigItem(String &name, IniConfig* cfg);
		
		~IniConfigItem() {
			if (nextItem) {
				delete nextItem;
			}
		}
		
        void setFloat(double v);

        void setInt(int v);

        void setDword(uint32_t v);

        void setBool(bool v);

        void setString(String v);

        void unset();

        String getLine();

        float getFloat(float def);

        int getInt(int def);

        uint32_t getDword(uint32_t def);

        bool getBool(bool def);

        String getString(String def);

        IniConfigItem* getNext();
    
        void setNext(IniConfigItem* item);

        IniConfigItem* cut();

        void setPrev(IniConfigItem* prev);

        bool isChanged();

        void unChanged();

    protected:
		enum ValueType {
			NONE,
			STRING,
			INT,
			FLOAT,
			DWORD,
            BOOL
		} 				type = NONE;

        union {
            int         valueInt;
            uint32_t    valueDword;
            double		valueFloat;
            bool        valueBool;
        };
        
		String			stringValue;
		bool 			changed = false;
		
		IniConfigItem*	nextItem = NULL;
        IniConfigItem*	prevItem = NULL;
		IniConfig*		config	= NULL;

        void set(ValueType type);
};

class IniConfigGroup {
	public:
		String name;

        IniConfigGroup(String name) {
            this->name = name;
        }

        IniConfigItem* findItem(String name);
		
		~IniConfigGroup();

        void cut();

        void add(IniConfigItem* item);

        void setNext(IniConfigGroup* next);

        void setPrev(IniConfigGroup* prev);

        bool write(File &file);

        bool isChanged();
		
        void dump();

        void unChanged();

        IniConfigGroup* getNext();

        IniConfigGroup* getPrev();

        IniConfigItem* getItems() {
            return items;
        }

    protected:
		IniConfigItem* items = NULL;
        IniConfigGroup* nextGroup = NULL;
        IniConfigGroup* prevGroup = NULL;
};

class IniConfig {
	public:
		IniConfig(String name, fs::FS* fs, bool doread = true) {
			this->filename = name;
			this->fs = fs;
			groups = NULL;
            if (doread) {
                this->read();
            }
		}
		
		~IniConfig() {
            clear();
		}

        void clear();

        bool isChanged();

        IniConfigGroup* findGroup(String name);

        IniConfigItem* findItem(String group, String name);

        IniConfigItem* findOrCreateItem(String group, String name);

        float stringToFloat(String &value, int pointAt) {
            float result = value.substring(0, pointAt).toInt();
            if (result > -1) {
                String right = value.substring(pointAt+1);
                uint8_t vlen = right.length();
                for (uint8_t p = 0; p < vlen; p++) {
                    uint8_t i = (int)right.charAt(p) - 48;
                    if (i > 0) {
                        result += ((double)i / ((double)pow(10, p+1)));
                    }
                }
            }
            return result;
        }

		bool read();
		
		bool write();

        bool update() {
            if (this->isChanged()) {
				Serial.println(F("writing...")); delay(2000);
                return this->write();
            } else {
				Serial.println(F("no update!")); delay(2000);
			}
            return true;
        }

        void removeGroup(String group);

        void add(IniConfigGroup* group);
		
		
        float getFloat(String group, String name, float def);

        int getInt(String group, String name, int def);

        uint32_t getDword(String group, String name, uint32_t def);

        bool getBool(String group, String name, bool def);

        String getString(String group, String name, String def);

        void setFloat(String group, String item, float value);

        void setInt(String group, String item, int value);

        void setDword(String group, String item, uint32_t value);

        void setBool(String group, String item, bool value);

        void setString(String group, String item, String value);

        void unset(String group, String name);
		
		void changed() {
			if (autoUpdate) {
				this->update();
			}
		}

        void unChanged() {
            if (this->groups) {
				groups->unChanged();
			}
        }
		
		void dump();
		
		void setAutoupdate(bool update) {
			this->autoUpdate = update;
		}

        IniConfigGroup* getGroups() {
            return groups;
        }

	protected:
		String			filename;
		IniConfigGroup*	groups = NULL;
		fs::FS*			fs;
		bool			autoUpdate = false;
};