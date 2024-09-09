#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "struct.h"
#include "hash.h"



int get_string( char* str ){
      int tmp = getchar_unlocked();
      int i = 0;

      if( tmp == EOF ){
            return END_OF_FILE;
      }
      
      while( tmp >= '0' ){
            str[i] = tmp;
            i++;
            tmp = getchar_unlocked();
      }
      str[i] = '\0';
      if( tmp == '\n' )
            return END_ROW;
      return CONTINUE;
}

int get_number( unsigned int* var ){
      int tmp = getchar_unlocked();
      *var = 0;

      if( tmp == EOF ){
            return END_OF_FILE;
      }
      
      while( tmp >= '0' && tmp <= '9' ){
            *var = (*var) * BASE + (tmp - '0');
            tmp = getchar_unlocked();
      }
      if( tmp == '\n' )
            return END_ROW;
      return CONTINUE;
}

unsigned long int* compress( char* str, char* length ){
      int len = 0;

      while( str[len] != '\0' ){
            len++;
      }

      unsigned long int* c = (unsigned long int*)malloc( sizeof(unsigned long int)*(len + ROUNDING)/MAX_STORED_DIGITS );

      for( int i = 0; i < (len + ROUNDING)/MAX_STORED_DIGITS; i++ ){
            int j = 0;
            unsigned long int mul = 1;
            c[i] = 0;

            while( j < MAX_STORED_DIGITS && str[i*MAX_STORED_DIGITS+j] != '\0' ){

                  c[i] += (long int)(((int)str[i*MAX_STORED_DIGITS+j]) - CHAR_OFFSET)*mul;
                  mul *= COMPRESS_BASE;
                  j++;
            }
      }

      *length = len;
      return c;
}

char* decompress( unsigned long int* comp, char len ){
      char* str = (char*)malloc((int)len + 1);
      int a_len = ((int)len + ROUNDING)/MAX_STORED_DIGITS;


      for( int i = 0; i < a_len; i++ ){
            unsigned long int tmp = comp[i];
            int j = 0;
            while( tmp > 0 ){
                  str[ i*MAX_STORED_DIGITS + j ] = (char)( tmp%COMPRESS_BASE + CHAR_OFFSET );
                  tmp /= COMPRESS_BASE;
                  j++;
            }
      }
      str[(int)len] = '\0';
      return str;
}

char compare_compressed_strings( unsigned long int* a, unsigned long int* b, int len ){
      for( int i = 0; i < len; i++ ){
            if( a[ len - 1 - i ] != b[ len - 1 - i ] ){
                  return 0;
            }
      }
      return 1;
}

unsigned int hash_string( char* ingredient ){
      unsigned int key = 1;
      int len = 0;
      int n_of_chars;

      while( ingredient[len] != '\0' ){
            len++;
      }
      len--;
      n_of_chars = len;
      
      if( len > MAX_UINT_ENCRYPT ){
            n_of_chars = MAX_UINT_ENCRYPT;
      }

      for ( int i = 0; i < n_of_chars; i++ ){
            key = key*COMPRESS_BASE +  ( ingredient[len - i] - CHAR_LIMIT );
      }

      return key;
}
