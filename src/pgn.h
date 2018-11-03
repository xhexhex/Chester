#ifndef PGN_H
#define PGN_H

#define SAN_MIN_LENGTH 2
#define SAN_MAX_LENGTH 7

Rawcode san_to_rawcode( const Pos *p, const char *san );
char *rawcode_to_san( const Pos *p, Rawcode rc, char promotion );

#endif
// end PGN_H
