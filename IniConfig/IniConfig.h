#pragma once

#include <Arduino.h>
#include <FS.h>

#define DEBUGLN(t) Serial.println(t);
#define DEBUG(t) Serial.print(t);

class IniConfigItem {
	public:
		String name;

        IniConfigItem(String &name) {
			this->name = name;
		}
		
		~IniConfigItem() {
			if (nextItem) {
				delete nextItem;
			}
		}

        void set(double v);

        void set(int v);

        void set(uint32_t v);

        void set(bool v);

        void set(String v);

        void unset();

        String getLine();

        void get(float &v);

        void get(int &v);

        void get(uint32_t &v);

        void get(bool &v);

        void get(String &v);

        IniConfigItem* getNext();
    
        void setNext(IniConfigItem* item);

        IniConfigItem* cut();

        void setPrev(IniConfigItem* prev);

        bool isChanged();

    protected:
		enum ValueType {
			NONE,
			STRING,
			INT,
			FLOAT,
			DWORD,
            BOOL
		} type = NONE;

        union {
            int         valueInt;
            uint32_t    valueDword;
            float       valueFloat;
            bool        valueBool;
        };
        
		String stringValue;
		bool changed;
		
		IniConfigItem* nextItem = NULL;
        IniConfigItem* prevItem = NULL;

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

        IniConfigGroup* getNext();

        IniConfigGroup* getPrev();

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

        IniConfigItem* findOrCreateItem(String &group, String &name);

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
            DEBUG("stringToFloat RESULT: ")
            DEBUGLN(result);
            return result;
        }

		bool read();
		
		bool write();

        bool update() {
            if (this->isChanged()) {
                DEBUG("IniConfig/update!");
                return this->write();
            }
            return true;
        }

        void setFloat(String group, String item, float value);

        void setInt(String group, String item, int value);

        void setDword(String group, String item, uint32_t value);

        void setBool(String group, String item, bool value);

        void setString(String group, String item, String value);

        /////

        float getFloat(String group, String name, float def);

        int getInt(String group, String name, int def);

        uint32_t getDword(String group, String name, uint32_t def);

        bool getBool(String group, String name, bool def);

        String getString(String group, String name, String def);

        void unset(String group, String name);

        void removeGroup(String group);

        void add(IniConfigGroup* group);

	protected:
		String filename;
		IniConfigGroup* groups = NULL;
		fs::FS* fs;
};