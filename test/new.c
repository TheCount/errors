#include<assert.h>
#include<stdio.h>

#include"../errors.h"

int main() {
	Error * e = error_newc( "Test!" );
	assert( e != NULL );
	int rc = error_frender( NULL, e, "\n", stdout );
	assert( rc >= 0 );
	error_del( e );
}
