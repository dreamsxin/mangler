/* Copyright 2010 Bob Shaffer II
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "inilib.h"

#include <sstream>
#include <fstream>
#include <cctype>
#include <cstdlib>

using namespace std;

bool iniCaselessCmp::operator()(const string &left, const string &right) const {
    string::const_iterator p = left.begin();
    string::const_iterator q = right.begin();

    while (p != left.end() && q != right.end() && tolower(*p) == tolower(*q)) {
        ++p; ++q;
    }
    if (p == left.end()) return q != right.end();
    if (q == right.end()) return false;
    return tolower(*p) < tolower(*q);
}

iniVariant::iniVariant() : mValue("") {}

iniVariant::iniVariant(const string &s) : mValue(s) {}

iniVariant::iniVariant(const char *s) : mValue(s) {}

iniVariant::iniVariant(int n) {
    ostringstream temp;
    temp << n;
    mValue = temp.str();
}

iniVariant::iniVariant(unsigned n) {
    ostringstream temp;
    temp << n;
    mValue = temp.str();
}

iniVariant::iniVariant(long n) {
    ostringstream temp;
    temp << n;
    mValue = temp.str();
}

iniVariant::iniVariant(unsigned long n) {
    ostringstream temp;
    temp << n;
    mValue = temp.str();
}

iniVariant::iniVariant(long long n) {
    ostringstream temp;
    temp << n;
    mValue = temp.str();
}

iniVariant::iniVariant(double d) {
    ostringstream temp;
    temp << d;
    mValue = temp.str();
}

iniVariant::iniVariant(bool b) {
    mValue = b ? "True" : "False";
}

#if ADD_GLIB_SUPPORT
iniVariant::iniVariant(const Glib::ustring &s) : mValue( Glib::locale_from_utf8(s) ) {}

Glib::ustring iniVariant::toUString() const {
    return Glib::locale_to_utf8(mValue);
}
#endif

int iniVariant::toInt() const {
    return atoi(mValue.c_str());
}

unsigned iniVariant::toUInt() const {
    return (unsigned)atol(mValue.c_str());
}

long iniVariant::toLong() const {
    return atol(mValue.c_str());
}

unsigned long iniVariant::toULong() const {
    return strtoul(mValue.c_str(), NULL, 10);
}

long long iniVariant::toLLong() const {
    return atoll(mValue.c_str());
}

double iniVariant::toDouble() const {
    return strtod(mValue.c_str(), NULL);
}

bool iniVariant::toBool() const {
    string temp = toLower();
    return (temp == "yes" || temp == "true" || temp == "1");
}

string::size_type iniVariant::length() const {
    return mValue.length();
}

iniVariant::operator string &() { return mValue; }

iniVariant::operator const string &() { return mValue; }

string iniVariant::toString() const { return mValue; }

string iniVariant::toUpper() const {
    string ret;
    int len = mValue.length();
    for (int i = 0; i < len; ++i) {
        char c = toupper(mValue[i]);
        ret += c;
    }
    return ret;
}

string iniVariant::toLower() const {
    string ret;
    int len = mValue.length();
    for (int i = 0; i < len; ++i) {
        char c = tolower(mValue[i]);
        ret += c;
    }
    return ret;
}

const char *iniVariant::toCString() const { return mValue.c_str(); }

iniVariant iniVariant::mNULLvariant;

iniValue::iniValue(const iniVariant &v) {
    append(v);
}

bool iniVariant::operator==(const iniVariant &v) const
    { return (mValue == v.mValue); }

iniValue::operator iniVariant &() {
    // make the vector size be 1, and return a reference to
    // that element
    if (size() == 0) insert(end(), iniVariant());
    if (size() > 1) erase(begin(), end() - 2);
    return at(0);
}

iniValue::operator const iniVariant &() {
    if (size() == 0) return iniVariant::null();
    return at(0);
}

iniVariant iniValue::value() const {
    if (size() == 0) return iniVariant();
    return at(0);
}

iniValue::size_type iniValue::count() const { return size(); }

void iniValue::append(const iniVariant &v) { push_back(v); }

iniValue &iniValue::operator=(const iniVariant &v)
    { ((iniVariant&)(*this)) = v; return (*this); }

iniValue &iniValue::operator+=(const iniVariant &v)
    { push_back(v); return (*this); }

bool iniValue::operator==(const iniVariant &v) const {
    return (size() == 1 && at(0) == v);
}

string iniValue::toString() const
    { return value().toString(); }

string iniValue::toUpper() const
    { return value().toUpper(); }

string iniValue::toLower() const
    { return value().toLower(); }

const char *iniValue::toCString() const
    { return value().toCString(); }

Glib::ustring iniValue::toUString() const
    { return value().toUString(); }

int iniValue::toInt() const
    { return value().toInt(); }

unsigned iniValue::toUInt() const
    { return value().toUInt(); }

long iniValue::toLong() const
    { return value().toLong(); }

unsigned long iniValue::toULong() const
    { return value().toULong(); }

long long iniValue::toLLong() const
    { return value().toLLong(); }

double iniValue::toDouble() const
    { return value().toDouble(); }

bool iniValue::toBool() const
    { return value().toBool(); }

string::size_type iniValue::length() const
    { return value().length(); }

bool iniSection::contains(const string &s) const {
    return (find(s) != end());
}

iniValue::size_type iniSection::count(const string &s) const {
    if (contains(s)) return at(s).size();
    else return 0;
}

ostream &iniSection::save(ostream &out) const {
    map<string, iniValue>::const_iterator iter;
    for (iter = begin(); iter != end(); iter++) {
        int vcnt = iter->second.size();
        for (int i = 0; i < vcnt; ++i)
            saveLine(out, iter->first, iter->second.at(i).toString());
    }
    return out;
}

ostream &iniSection::saveLine(ostream &out, const string &keyName, const string &value) {
    // this is lame, and i've never seen it used outside of mysql's config
    //if (value.empty()) {
    //    out << quoteString(keyName) << endl;
    //} else {
    out << quoteString(keyName) << " = " << quoteString(value) << endl;
    //}
    return out;
}

string iniSection::quoteString(const string &s) {
    string ret;
    int len = s.length();
    bool has_space( false );
    for (int i = 0; i < len; ++i) {
        char c( s[i] );
        if (c == ' ' || c == '\t') has_space = true;
        if (c == '\"') ret.append("\\");
        ret += c;
    }
    if (has_space) {
        ret.insert(0, "\"");
        ret.append("\"");
    }
    return ret;
}

void iniSection::trimString(string &s) {
    if (s.empty()) return;
    while (s.length() && (s[0] == ' ' || s[0] == '\t')) s.erase(0, 1);
    if (s.empty()) return;
    long right = (long)(s.length()) - 1l;
    while (right > -1l && (s[right] == ' ' || s[right] == '\t')) right--;
    if (right < (long)(s.length()) - 1l) s.erase(right, s.npos);
}

vector<string> iniSection::tokenizeString(const string &s) {
    vector<string> ret;
    string temp;
    int len( s.length() );
    if (! len) return ret;
    int i = 0;
    while (i < len) {
        if (s[i] == '\"') {
            while (i < len) {
                if (s[i] == '\\') {
                    ++i; if (i < len - 1) temp += s[i];
                } else if (s[i] == '\"') break;
                else temp += s[i];
                ++i;
            }
            trimString(temp);
            if (temp.length()) ret.push_back(temp);
            temp.clear();
        } else if (s[i] == '\t' || s[i] == ' ') {
            trimString(temp);
            if (temp.length()) ret.push_back(temp);
            temp.clear();
        } else if (s[i] == '=') {
            trimString(temp);
            if (temp.length()) ret.push_back(temp);
            temp.clear();
            ret.push_back("=");
        } else temp += s[i];
        ++i;
    }
    trimString(temp);
    if (temp.length()) ret.push_back(temp);
    return ret;
}

vector<string> iniSection::parseLine(const string &s) {
    vector<string> ret, tokens = tokenizeString(s);
    // allow sloppy ini files
    int i = 1, len = tokens.size();
    if (! len || tokens[0] == "=") return ret;
    ret.push_back(tokens[0]);
    while (i < len && tokens[i] != "=") {
        ret[0].append(" ");
        ret[0].append(tokens[i]);
        ++i;
    }
    ++i; // skip the =
    if (i < len) {
        ret.push_back(tokens[i]);
        ++i;
    } else ret.push_back("");
    while (i < len) {
        ret[1].append(" ");
        ret[1].append(tokens[i]);
        ++i;
    }
    return ret;
}

iniFile::iniFile(const string &filename) : mFilename( filename ) {
    reload();
}

void iniFile::setFilename(const string &filename)
    { mFilename = filename; }

string iniFile::getFilename() const
    { return mFilename; }

istream &iniFile::load(istream &in) {
    // this is the big ugly shit that reads the file
    clear();
    string curSect( "" );
    string temp;
    for (;;) {
        getline(in, temp);
        if (in.eof()) break;
        cleanLine(temp);
        if (temp.empty()) continue;
        if (temp[0] == '[') {
            // new section
            removeBrackets(temp);
            curSect = temp;
        } else {
            vector<string> kvPair( iniSection::parseLine(temp) );
            if (kvPair.size() == 2 && ! curSect.empty()) 
                (*this)[curSect][kvPair[0]].append(kvPair[1]);
            // else print parse error message
        }
    }
    return in;
}

ostream &iniFile::save(ostream &out) const {
    map<string, iniSection>::const_iterator iter;
    for (iter = begin(); iter != end(); iter++) {
        out << '[' << iter->first << ']' << endl;
        iter->second.save(out);
        out << endl;
    }
    return out;
}

void iniFile::save() const {
    if (! mFilename.empty()) {
        ofstream fout( mFilename.c_str() );
        if (fout) save(fout);
        fout.close();
    }
}

void iniFile::reload() {
    ifstream fin( mFilename.c_str() );
    if (fin) {
        load(fin);
        fin.close();
    }
}

iniFile::~iniFile() {
    save();
}

bool iniFile::contains(const string &s) const {
    return (find(s) != end());
}

void iniFile::cleanLine(string &s) {
    int len = s.length();
    bool in_quotes( false );
    for (int i = 0; i < len; ++i) {
        if (s[i] == '\"') in_quotes = ! in_quotes;
        else if (s[i] == '#' || s[i] == ';') {
            s.erase(i, s.npos);
            break;
        }
    }
    iniSection::trimString(s);
}

void iniFile::removeBrackets(string &s) {
    // assume string is trimmed already and has brackets around it
    s.erase(0, 1);
    s.erase(s.length() - 1, s.npos);
    iniSection::trimString(s);
}

istream &operator>>(istream &in, iniFile &f) { return f.load(in); }
ostream &operator<<(ostream &out, iniFile &f) { return f.save(out); }

