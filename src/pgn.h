#ifndef PGN_H
#define PGN_H

Rawcode san_to_rawcode( const Pos *p, const char *san );
char *rawcode_to_san( const Pos *p, Rawcode rc );

#endif
// end PGN_H
