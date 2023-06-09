/*=========================================================================
Module    : AneuSurfIsolation
File      : aneuConfigFile.h
Copyright : (C)opyright 2009++
            See COPYRIGHT statement in top level directory.
Authors   : Richard J. Wagner
Modified  : Daniel Millan
Purpose   : Class for reading named values from configuration files
Date      :
Version   :
Changes   :

    This software is distributed WITHOUT ANY WARRANTY; without even 
    the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
    PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#include "aneuConfigFile.h"

using std::string;

aneuConfigFile::aneuConfigFile( string filename, string delimiter,
                        string comment, string sentry )
    : myDelimiter(delimiter), myComment(comment), mySentry(sentry)     //  se usa ':' como constructor, un método especial que se llama al crear el objeto de la clase
{
    // Construct a aneuConfigFile, getting keys and values from given file
    
    std::ifstream in( filename.c_str() );
    
    if( !in )
    {
        std::cout << "ERROR::ConfigFile - File '" << filename << "' not found.";
        std::cout << std::endl;
        throw file_not_found ( filename );
    }
    in >> (*this);
}


aneuConfigFile::aneuConfigFile()
    : myDelimiter( string(1,'=') ), myComment( string(1,'#') )
{
    // Construct a aneuConfigFile without a file; empty
}

void aneuConfigFile::remove( const string& key )
{
    // Remove key and its value
    this->myContents.erase( this->myContents.find( key ) );
    return;
}


bool aneuConfigFile::keyExists( const string& key ) const
{
    // Indicate whether key is found
    mapci p = this->myContents.find( key );
    return ( p != this->myContents.end() );
}


// static //
void aneuConfigFile::trim( string& s )
{
    // Remove leading and trailing whitespace
    static const char whitespace[] = " \n\t\v\r\f";
    s.erase( 0, s.find_first_not_of(whitespace) );
    s.erase( s.find_last_not_of(whitespace) + 1U );
}


std::ostream& operator<<( std::ostream& os, const aneuConfigFile& cf )
{
    // Save a aneuConfigFile to os
    for( aneuConfigFile::mapci p = cf.myContents.begin();
        p != cf.myContents.end();
        ++p )
    {
        os << p->first << " " << cf.myDelimiter << " ";
        os << p->second << std::endl;
    }
    return os;
}


std::istream& operator>>( std::istream& is, aneuConfigFile& cf )
{
    // Load a aneuConfigFile from is
    // Read in keys and values, keeping internal whitespace
    typedef string::size_type pos;
    const string& delim  = cf.myDelimiter;  // separator
    const string& comm   = cf.myComment;    // comment
    const string& sentry = cf.mySentry;     // end of file sentry
    const pos skip = delim.length();        // length of separator
    
    string nextline = "";  // might need to read ahead to see where value ends
    
    while( is || nextline.length() > 0 )
    {
        // Read an entire line at a time
        string line;
        if( nextline.length() > 0 )
        {
            line = nextline;  // we read ahead; use it now
            nextline = "";
        }
        else
        {
            std::getline( is, line );
        }
        
        // Ignore comments
        line = line.substr( 0, line.find(comm) );
        
        // Check for end of file sentry
        if( sentry != "" && line.find(sentry) != string::npos ) return is;
        
        // Parse the line if it contains a delimiter
        pos delimPos = line.find( delim );
        if( delimPos < string::npos )
        {
            // Extract the key
            string key = line.substr( 0, delimPos );
            line.replace( 0, delimPos+skip, "" );
            
            // See if value continues on the next line
            // Stop at blank line, next line with a key, end of stream,
            // or end of file sentry
            bool terminate = false;
            while( !terminate && is )
            {
                std::getline( is, nextline );
                terminate = true;
                
                string nlcopy = nextline;
                aneuConfigFile::trim(nlcopy);
                if( nlcopy == "" ) continue;
                
                nextline = nextline.substr( 0, nextline.find(comm) );
                if( nextline.find(delim) != string::npos )
                    continue;
                if( sentry != "" && nextline.find(sentry) != string::npos )
                    continue;
                
                nlcopy = nextline;
                aneuConfigFile::trim(nlcopy);
                if( nlcopy != "" ) line += "\n";
                line += nextline;
                terminate = false;
            }
            
            // Store key and value
            aneuConfigFile::trim(key);
            aneuConfigFile::trim(line);
            cf.myContents[key] = line;  // overwrites if key is repeated
        }
    }
    
    return is;
}
