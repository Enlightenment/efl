#ifndef SGFONTDATABASE_H
#define SGFONTDATABASE_H

#include"sgglobal.h"

class SGFontDatabasePrivate;
class SG_EXPORT SGFontDatabase
{
public:
    enum HintMode{
        Default,
        None,
        Auto,
        Full
    };
    ~SGFontDatabase();
    static SGFontDatabase &instance()
    {
        static SGFontDatabase Singleton;
        return Singleton;
    }
    SGFontDatabase(const SGFontDatabase &other) = delete;
    SGFontDatabase(SGFontDatabase&&) = delete;
    SGFontDatabase& operator=(SGFontDatabase const&) = delete;
    SGFontDatabase& operator=(SGFontDatabase &&) = delete;
    void print() const;
private:
    SGFontDatabase();
    friend class SGFontDatabasePrivate;
    SGFontDatabasePrivate *d;
};

#endif // SGFONTDATABASE_H
