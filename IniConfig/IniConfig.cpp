#include "IniConfig.h"
#include "FS.h"

IniConfigItem::IniConfigItem(String &name, IniConfig* cfg) {
			this->name = name;
			this->config = cfg;
			this->set(NONE);
		}

void IniConfigItem::setFloat(double v) {
    this->valueFloat = v;
    set(FLOAT);
			config->changed();
}

void IniConfigItem::setInt(int v) {
    this->valueInt = v;
    set(INT);
			config->changed();
}

void IniConfigItem::setDword(uint32_t v) {
    this->valueDword = v;
    set(DWORD);
			config->changed();
}

void IniConfigItem::setBool(bool v) {
    this->valueBool = v;
    set(BOOL);
			config->changed();
}

void IniConfigItem::setString(String v) {
    this->stringValue = v;
    set(STRING);
			config->changed();
}

void IniConfigItem::unset() {
    this->stringValue = F("");
    set(NONE);
    cut();
			config->changed();
}

String IniConfigItem::getLine() {
    this->changed = false;
    String result = this->name + String('=');
    switch (this->type)
    {
        case STRING:
            return result + stringValue;
        case DWORD:
            return result + String(valueDword);
        case INT:
            return result + String(valueInt);
        case FLOAT:
            return result + String(valueFloat);
        case BOOL:
            return result + valueBool ? String(F("true")) : String(F("false"));
        case NONE: {}
    }
    return String(F(""));
}

float IniConfigItem::getFloat(float def) {
    if (this->type == FLOAT) {
        return this->valueFloat;
    } else if (this->type == DWORD) {
        return (float)this->valueDword;
    } else if (this->type == INT) {
        return (float)this->valueInt;
    } else if (this->type == STRING) {
        return this->stringValue.toFloat();
    }
			return def;
}

int IniConfigItem::getInt(int def) {
    if (this->type == INT) {
        return this->valueInt;
    } else if (this->type == FLOAT) {
        return (int)this->valueFloat;
    } else if (this->type == DWORD) {
        return (int)this->valueDword;
    } else if (this->type == STRING) {
        return this->stringValue.toInt();
    }
			return def;
}

uint32_t IniConfigItem::getDword(uint32_t def) {
    if (this->type == DWORD) {
				return this->valueDword;
    } else if (this->type == FLOAT) {
        return (uint32_t)this->valueFloat;
    } else if (this->type == INT) {
        return (uint32_t)this->valueInt;
    } else if (this->type == STRING) {
        return this->stringValue.toInt();
    }
			return def;
}

bool IniConfigItem::getBool(bool def) {
    if (this->type == BOOL) {
        return this->valueBool;
    } else if (this->type == DWORD) {
        return this->valueDword != 0;
    } else if (this->type == INT) {
        return this->valueInt != 0;
    } else if (this->type == STRING) {
        return this->stringValue.equalsIgnoreCase(F("true"));
    }
    return def;
}

String IniConfigItem::getString(String def) {
    if (this->type == STRING) {
        return this->stringValue;
    } if (this->type == BOOL) {
        return String(this->valueBool);
    } else if (this->type == DWORD) {
        return String(this->valueDword);
    } else if (this->type == INT) {
        return String(this->valueInt);
    }
			return def;
}

IniConfigItem* IniConfigItem::getNext() {
    return this->nextItem;
}

void IniConfigItem::setNext(IniConfigItem* item) {
    this->nextItem = item;
}

IniConfigItem* IniConfigItem::cut() {
    if (prevItem) {
        prevItem->setNext(nextItem ? nextItem : NULL);
    }
    this->setNext(NULL);
    this->setPrev(NULL);
    return this;
}

void IniConfigItem::setPrev(IniConfigItem* prev) {
    this->setPrev(prev);
    if (prev)
        prev->setNext(this);
}

bool IniConfigItem::isChanged() {
    return this->changed;
}

void IniConfigItem::unChanged() {
    this->changed = false;
}

void IniConfigItem::set(ValueType type) {
    this->type = type;
    this->changed = true;
}

IniConfigItem* IniConfigGroup::findItem(String name) {
    IniConfigItem* item = items;
    //DEBUGLN("group/fingItem")
    while (item) {
        //DEBUG("group/fingItem/check: ");
        //DEBUGLN(item->name);
        if (item->name.equalsIgnoreCase(name)) {
            //DEBUGLN("group/fingItem/found")
            return item;
        }
        item = item->getNext();
    }
    return NULL;
}

IniConfigGroup::~IniConfigGroup() {
	if (items) {
		delete items;
	}
            if (nextGroup) {
                delete nextGroup;
            }
}

void IniConfigGroup::cut() {
    if (prevGroup) {
        prevGroup->setNext( prevGroup ? prevGroup : NULL );
    }
    this->prevGroup = NULL;
    this->nextGroup = NULL;
}

void IniConfigGroup::add(IniConfigItem* item) {
    if (!items) {
        //DEBUGLN("item/add/1");
        items = item;
        return;
    }
    IniConfigItem* prev = items;
    while (prev && prev->getNext()) {
        //DEBUGLN("item/add/2-");
        prev = prev->getNext();
    }
    prev->setNext(item);
}

void IniConfigGroup::setNext(IniConfigGroup* next) {
    this->nextGroup = next;
}

void IniConfigGroup::setPrev(IniConfigGroup* prev) {
    this->prevGroup = prev;
    if (prev)
        prev->setNext(this);
}

bool IniConfigGroup::write(File &file) {
    if (items) {
        file.print(F("["));
        file.print(this->name);
        file.println(F("]"));
        IniConfigItem* item = items;
        while (item != NULL) {
            String line = item->getLine();
            if (line.length() > 0) {
                file.println(line);
            }
            item = item->getNext();
        }
    }
    return true;
}

bool IniConfigGroup::isChanged() {
    IniConfigItem* item = items;
			//Serial.println(String(F("group: ")) + this->name + F(" changed?"));
    while (item != NULL) {
				//Serial.println(String(F("Item: ")) + item->name + String(F(" changed?")) + String(item->isChanged()));
        if (item->isChanged()) {
            return true;
        }
        item = item->getNext();
    }
    if (this->getNext()) {
        return this->getNext()->isChanged();
    }
    return false;
}

void IniConfigGroup::dump() {
    IniConfigItem* item = items;
			Serial.println(String(F("== ")) + this->name + String(F(" ==")));
    while (item) {
				Serial.println(String(F("- ")) + item->name + String(item->isChanged() ? F(" +=") : F(" = ") ) + item->getString(F("")));
        item = item->getNext();
    }
    if (this->getNext()) {
        this->getNext()->dump();
    }
}

void IniConfigGroup::unChanged() {
    IniConfigItem* item = items;
    while (item) {
        item->unChanged();
        item = item->getNext();
    }
    if (this->getNext()) {
        this->getNext()->unChanged();
    }
}

IniConfigGroup* IniConfigGroup::getNext() {
    return nextGroup;
}

IniConfigGroup* IniConfigGroup::getPrev() {
    return prevGroup;
}

void IniConfig::clear() {
    if (groups) {
				delete groups;
			}
}

bool IniConfig::isChanged() {
    if (this->groups) {
        return groups->isChanged();
    }
    return false;
}

IniConfigGroup* IniConfig::findGroup(String name) {
    IniConfigGroup* group = groups;
    while (group) {
        if (group->name.equalsIgnoreCase(name)) {
            return group;
        }
        group = group->getNext();
    }
    return NULL;
}

IniConfigItem* IniConfig::findItem(String group, String name) {
    IniConfigGroup* g = this->findGroup(group);
    if (!g) {        
        return NULL;
    }
    return g->findItem(name);
}

IniConfigItem* IniConfig::findOrCreateItem(String group, String name) {
    IniConfigGroup* g = this->findGroup(group);
    IniConfigItem* item = NULL;
    if (g) {
        item = g->findItem(name);
    } else {
        g = new IniConfigGroup(group);
        this->add(g);
    }
    if (!item) {
        item = new IniConfigItem(name, this);
        g->add(item);
    }
    return item;
}

bool IniConfig::read() {
            clear();
            File file = fs->open(this->filename, "r");
            if (file) {
                IniConfigGroup* curGroup = NULL;
                while (file.available()) {
                    String line = file.readStringUntil('\n');
                    line.trim();
                    if (!line.isEmpty()) {
                        //DEBUGLN("not empt");
                        char c = line.charAt(0);
                        if (c == '[') {
                            //DEBUGLN("group!");
                            // group
                            if (line.charAt(line.length()-1) == ']') {
                                //DEBUGLN("GROUP::");
                                //DEBUGLN(line.substring(1, line.length()-1));
                                String name = line.substring(1, line.length()-1);
                                name.trim();
                                IniConfigGroup* group = new IniConfigGroup(name);
                                this->add(group);
                                curGroup = group;
                                //DEBUGLN(String(F("new group: ")) + name);
                            }
                            continue;
                        }
                        uint16_t delimPos = line.indexOf('=');
                        if (curGroup && delimPos > 0) {
                            String paramName = line.substring(0, delimPos);
                            paramName.trim();
                            String value = line.substring(delimPos+1);
                            value.trim();
                            IniConfigItem* item = curGroup->findItem(paramName);
                            if (!item) {
                                item = new IniConfigItem(paramName, this);
                                curGroup->add(item);
                            }
                            if (value.equalsIgnoreCase(F("true")) || value.equalsIgnoreCase(F("false")) ) {
                                item->setBool(value.equalsIgnoreCase(F("true")));
                            } else {
                                if (value.length() < 8) {
                                    bool decimals = true;
                                    for (uint16_t i = 0; i < value.length(); i++) {
                                        char c = value.charAt(i);
                                        if (c != '-' && c != '.' && !(c >= '0' && c <= '9')) {
                                            decimals = false;
                                            break;
                                        }
                                    }
                                    if (decimals) {
                                        int pointAt = value.indexOf('.');
                                        //num
                                        if (value.charAt(0) == '-') {
                                            if (pointAt == -1) {
                                                item->setInt( -1 * value.substring(1).toInt() );
                                            } else {
                                                value = value.substring(1);
                                                item->setFloat( -1.0 * stringToFloat(value, pointAt-1));
                                            }
                                        } else {
                                            if (pointAt != -1) {
                                                item->setFloat(stringToFloat(value, pointAt));
                                            } else {
                                                uint32_t v = 0;
                                                uint8_t vlen = value.length();
                                                for (uint8_t p = 0; p < vlen; p++) {
                                                    v += value.substring(vlen - 1 - p, vlen - p).toInt() * pow(10, p);
                                                }
                                                item->setDword(v);
                                            }
                                        }
                                    } else {
                                        item->setString(value);
                                    }
                                } else {
                                    item->setString(value);
                                }
                            }
                        }
                    }
                }
                file.close();
                this->unChanged();
            }
            return true;
}

bool IniConfig::write() {
            IniConfigGroup* group = groups;
            File file = fs->open(this->filename, "w");
            if (!file) {
                return false;
            }
            bool result = true;
            while (group != NULL) {
                if (group->write(file)) {
                    group = group->getNext();
                    if (group) {
                        file.println();
                    }
                } else {
                    result = false;
                    break;
                }
            }
            file.close();
            this->unChanged();
            return result;
}

void IniConfig::removeGroup(String group) {
    IniConfigGroup* g = findGroup(group);
    if (g) {
        g->cut();
        delete g;
    }
}

void IniConfig::add(IniConfigGroup* group) {
    if (!groups) {
        groups = group;
        return;
    }
    IniConfigGroup* prev = groups;
    //DEBUGLN(String(F("add/check gr: ")) + prev->name);
    while (prev && prev->getNext()) {
        //DEBUGLN(String(F("add/check gr: ")) + prev->name);
        prev = prev->getNext();
    }
    //DEBUGLN(String(F("add/set prev gr: ")) + prev->name);
    group->setPrev(prev);
}

float IniConfig::getFloat(String group, String name, float def) {
    IniConfigItem* item = findItem(group, name);
    return item ? item->getFloat(def) : def;
}

int IniConfig::getInt(String group, String name, int def) {
    IniConfigItem* item = findItem(group, name);
    return item ? item->getInt(def) : def;
}

uint32_t IniConfig::getDword(String group, String name, uint32_t def) {
    IniConfigItem* item = findItem(group, name);
    return item ? item->getDword(def) : def;
}

bool IniConfig::getBool(String group, String name, bool def) {
    IniConfigItem* item = findItem(group, name);
    return item ? item->getBool(def) : def;
}

String IniConfig::getString(String group, String name, String def) {
    IniConfigItem* item = findItem(group, name);
    return item ? item->getString(def) : def;
}

void IniConfig::setFloat(String group, String item, float value) {
    findOrCreateItem(group, item)->setFloat(value);
}

void IniConfig::setInt(String group, String item, int value) {
    findOrCreateItem(group, item)->setInt(value);
}

void IniConfig::setDword(String group, String item, uint32_t value) {
    findOrCreateItem(group, item)->setDword(value);
}

void IniConfig::setBool(String group, String item, bool value) {
    findOrCreateItem(group, item)->setBool(value);
}

void IniConfig::setString(String group, String item, String value) {
    findOrCreateItem(group, item)->setString(value);
}

void IniConfig::unset(String group, String name) {
    IniConfigItem* item = findItem(group, name);
			if (item) {
				item->unset();
				delete item;
			}
}

void IniConfig::dump() {
	if (this->groups) {
		Serial.println(F("==[ CONFIG ]=="));
		groups->dump();
	}
}
