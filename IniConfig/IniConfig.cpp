#include "IniConfig.h"
#include "FS.h"

void IniConfigItem::set(double v) {
    DEBUG("IniConfigItem::setF, new value=")
    DEBUGLN(v)
    this->valueFloat = v;
    set(FLOAT);
}

void IniConfigItem::set(int v) {
    DEBUG("IniConfigItem::setI, new value=")
    DEBUGLN(v)
    this->valueInt = v;
    set(INT);
}

void IniConfigItem::set(uint32_t v) {
    DEBUG("IniConfigItem::setDw, new value=")
    DEBUGLN(v)
    this->valueDword = v;
    set(DWORD);
}

void IniConfigItem::set(bool v) {
    this->valueBool = v;
    set(BOOL);
}

void IniConfigItem::set(String v) {
    this->stringValue = v;
    set(STRING);
}

void IniConfigItem::unset() {
    this->stringValue = F("");
    set(NONE);
    cut();
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

void IniConfigItem::get(float &v) {
    if (this->type == FLOAT) {
        v = this->valueFloat;
    } else if (this->type == DWORD) {
        v = (float)this->valueDword;
    } else if (this->type == INT) {
        v = (float)this->valueInt;
    } else if (this->type == STRING) {
        v = this->stringValue.toFloat();
    }
}

void IniConfigItem::get(int &v) {
    if (this->type == INT) {
        v = this->valueInt;
    } else if (this->type == FLOAT) {
        v = (int)this->valueFloat;
    } else if (this->type == DWORD) {
        v = (int)this->valueDword;
    } else if (this->type == STRING) {
        v = this->stringValue.toInt();
    }
}

void IniConfigItem::get(uint32_t &v) {
    if (this->type == DWORD) {
        v = this->valueDword;
    } else if (this->type == FLOAT) {
        v = (uint32_t)this->valueFloat;
    } else if (this->type == INT) {
        v = (uint32_t)this->valueInt;
    } else if (this->type == STRING) {
        v = this->stringValue.toInt();
    }
}

void IniConfigItem::get(bool &v) {
    if (this->type == BOOL) {
        v = this->valueBool;
    } else if (this->type == DWORD) {
        v = this->valueDword != 0;
    } else if (this->type == INT) {
        v = this->valueInt != 0;
    } else if (this->type == STRING) {
        v = this->stringValue.equalsIgnoreCase(F("true"));
    }
    
}

void IniConfigItem::get(String &v) {
    if (this->type == STRING) {
        v = this->stringValue;
    } if (this->type == BOOL) {
        v = String(this->valueBool);
    } else if (this->type == DWORD) {
        v = String(this->valueDword);
    } else if (this->type == INT) {
        v = String(this->valueInt);
    }
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
    //DEBUGLN("item/add");
    if (!items) {
        //DEBUGLN("item/add/1");
        items = item;
        return;
    }
    //DEBUGLN("item/add/2");
    IniConfigItem* prev = items;
    while (prev && prev->getNext()) {
        //DEBUGLN("item/add/2-");
        prev = prev->getNext();
    }
    //DEBUGLN("item/add/3");
    //delay(2000);
    //item->setPrev(prev);
    //delay(2000);
    //DEBUGLN("item/add/4");
    prev->setNext(item);
    //delay(5000);
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
    //DEBUGLN("IniConfigGroup/write");
    if (items) {
        //DEBUG("IniConfigGroup/group!");
        //DEBUG(this->name);
        file.print(F("["));
        file.print(this->name);
        file.println(F("]"));
        IniConfigItem* item = items;
        while (item != NULL) {
            //DEBUG("IniConfigGroup/write item!");
            String line = item->getLine();
            if (line.length() > 0) {
                //file.prinln(line);
                /*file.write((const uint8_t*)line.c_str(), line.length());
                file.write('\r');
                file.write('\n');*/
                file.println(line);
            }
            //DEBUG("IniConfigGroup/write next");
            item = item->getNext();
        }
    }
    //DEBUG("IniConfigGroup/write done");
    return true;
}

bool IniConfigGroup::isChanged() {
    IniConfigItem* item = items;
    while (item != NULL) {
        if (item->isChanged()) {
            return true;
        }
        item = item->getNext();
    }
    return false;
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
    //DEBUGLN("IniConfig/findGroup");
    IniConfigGroup* group = groups;
    while (group) {
        /*DEBUG("IniConfig/while/group: ");
        DEBUG(group->name);
        DEBUG(" -?- ");
        DEBUGLN(name);*/
        if (group->name.equalsIgnoreCase(name)) {
            //DEBUGLN("findGroup!ok");
            return group;
        }
        group = group->getNext();
    }
    return NULL;
}

IniConfigItem* IniConfig::findItem(String group, String name) {
    //DEBUGLN("findItem");
    IniConfigGroup* g = this->findGroup(group);
    if (!g) {        
        return NULL;
    }
    //DEBUGLN("findItem/findGroup/get item");
    return g->findItem(name);
}

IniConfigItem* IniConfig::findOrCreateItem(String &group, String &name) {
    //DEBUGLN("findOrCreateItem");
    IniConfigGroup* g = this->findGroup(group);
    IniConfigItem* item = NULL;
    if (g) {
        item = g->findItem(name);
    } else {
        //DEBUGLN("findOrCreateItem//new group");
        g = new IniConfigGroup(group);
        //DEBUGLN("findOrCreateItem//new group/add");
        this->add(g);
    }
    if (!item) {
        //DEBUGLN("findOrCreateItem//new item");
        item = new IniConfigItem(name);
        //DEBUGLN("findOrCreateItem//new item/add");
        delay(2000);
        //DEBUGLN("findOrCreateItem//new item/add1");
        g->add(item);
    }
    return item;
}

bool IniConfig::read() {
            clear();
            File file = fs->open(this->filename, "r");
            //DEBUGLN("File read open");
            if (file) {
                IniConfigGroup* curGroup = NULL;
                //DEBUGLN("av?");
                while (file.available()) {
                    //DEBUGLN("av!");
                    String line = file.readStringUntil('\n');
                    line.trim();
                    //DEBUGLN("===========");
                    //DEBUGLN(line);
                    //DEBUGLN("===========");
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
                            }
                            //DEBUGLN("continue");
                            continue;
                        }
                        //DEBUGLN("not gr");
                        uint16_t delimPos = line.indexOf('=');
                        if (curGroup && delimPos > 0) {
                            //DEBUGLN("param!");
                            String paramName = line.substring(0, delimPos);
                            paramName.trim();
                            String value = line.substring(delimPos+1);
                            value.trim();
                            //DEBUGLN(paramName);
                            //DEBUGLN(value);
                            
                            //IniConfigItem* item = this->findItem(curGroup->name, paramName);
                            IniConfigItem* item = curGroup->findItem(paramName);
                            if (!item) {
                                item = new IniConfigItem(paramName);
                                curGroup->add(item);
                            }
                            if (value.equalsIgnoreCase(F("true")) || value.equalsIgnoreCase(F("false")) ) {
                                item->set(value.equalsIgnoreCase(F("true")));
                            } else {
                                //DEBUGLN("value?");
                                //DEBUGLN(value.length());
                                if (value.length() < 8) {
                                    bool decimals = true;
                                    for (uint16_t i = 0; i < value.length(); i++) {
                                        char c = value.charAt(i);
                                        if (c != '-' && c != '.' && !(c >= '0' && c <= '9')) {
                                            //DEBUG("found not decimal: ");
                                            //DEBUGLN(c);
                                            decimals = false;
                                            break;
                                        }
                                    }
                                    if (decimals) {
                                        //DEBUGLN("decimals!");
                                        int pointAt = value.indexOf('.');
                                        //num
                                        if (value.charAt(0) == '-') {
                                            if (pointAt == -1) {
                                                item->set( (int) (-1 * value.substring(1).toInt()) );
                                            } else {
                                                value = value.substring(1);
                                                item->set( (float) -1.0 * stringToFloat(value, pointAt-1));
                                            }
                                        } else {
                                            if (pointAt != -1) {
                                                item->set(stringToFloat(value, pointAt));
                                            } else {
                                                uint32_t v = 0;
                                                uint8_t vlen = value.length();
                                                for (uint8_t p = 0; p < vlen; p++) {
                                                    v += value.substring(vlen - 1 - p, vlen - p).toInt() * pow(10, p);
                                                }
                                                item->set(v);
                                            }
                                        }
                                    } else {
                                        //DEBUGLN("string!");
                                        item->set(value);
                                    }
                                } else {
                                    //DEBUGLN("string2");
                                    item->set(value);
                                }
                            }
                        }
                    }
                }
                file.close();
                //DEBUGLN("file close");
            }
            //DEBUGLN("read done");
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
            return result;
}

void IniConfig::setFloat(String group, String item, float value) {
    findOrCreateItem(group, item)->set(value);
}

void IniConfig::setInt(String group, String item, int value) {
    findOrCreateItem(group, item)->set(value);
}

void IniConfig::setDword(String group, String item, uint32_t value) {
    findOrCreateItem(group, item)->set(value);
}

void IniConfig::setBool(String group, String item, bool value) {
    findOrCreateItem(group, item)->set(value);
}

void IniConfig::setString(String group, String item, String value) {
    findOrCreateItem(group, item)->set(value);
}

float IniConfig::getFloat(String group, String name, float def) {
    float result = def;
    IniConfigItem* item = findItem(group, name);
    if (item) {
        item->get(result);
    }
    return result;
}

int IniConfig::getInt(String group, String name, int def) {
    int result = def;
    DEBUGLN(result);
    IniConfigItem* item = findItem(group, name);
    if (item) {
        item->get(result);
    }
    return result;
}

uint32_t IniConfig::getDword(String group, String name, uint32_t def) {
    uint32_t result = def;
    IniConfigItem* item = findItem(group, name);
    if (item) {
        item->get(result);
    }
    return result;
}

bool IniConfig::getBool(String group, String name, bool def) {
    bool result = def;
    IniConfigItem* item = findItem(group, name);
    if (item) {
        item->get(result);
    }
    return result;
}

String IniConfig::getString(String group, String name, String def) {
    String result = def;
    IniConfigItem* item = findItem(group, name);
    if (item) {
        item->get(result);
    }
    return result;
}

void IniConfig::unset(String group, String name) {
    IniConfigItem* item = findOrCreateItem(group, name);
    item->unset();
    delete item;
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
    while (prev && prev->getNext()) {
        prev = prev->getNext();
    }
    prev->setNext(group);
}
