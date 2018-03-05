#ifndef ERRORS_H_
#define ERRORS_H_

#include<stdarg.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

/**
 * Format check magic for GCC.
 */
#ifdef __GNUC__
#	define Error_FORMAT_CHECK( fmtidx, argsidx ) __attribute__( ( format ( printf, ( fmtidx ), ( argsidx ) ) ) )
#else
#	define Error_FORMAT_CHECK( fmtidx, argsidx ) /* no check */
#endif

/**
 * Bit determining whether an error has to be freed.
 */
#define Error_FREE ( ( uintptr_t ) 0x0001u )

/**
 * Bit determining whether an error message has to be freed.
 */
#define Error_MSG_FREE ( ( uintptr_t ) 0x0002u )

/**
 * Maximum size of an error message, including the terminating zero byte.
 */
#define Error_MAXLEN 1024u

/**
 * Error structure.
 */
typedef struct Error {
	/**
	 * Pointer to error message.
	 */
	char * msg;

	/**
	 * Pointer to wrapped error.
	 * If there is no wrapped error, this is a null pointer.
	 * LSBs are used to indicate whether the containing error has to be freed, and whether the message has to be freed.
	 */
	uintptr_t wrapped;
} Error;

/**
 * Out of memory error.
 */
static Error Error_NOMEM_ = {
	.msg = "Out of memory",
	.wrapped = 0,
};

/**
 * Empty error message error.
 */
static Error Error_EMPTY_ = {
	.msg = "<Empty>",
	.wrapped = 0,
};

/**
 * Allocation function.
 */
static void * ( *error_alloc_ )( size_t ) = malloc;

/**
 * Release function.
 */
static void ( *error_free_ )( void * ) = free;

/**
 * Obtains the wrapped error from an error.
 *
 * @param e Pointer to error.
 *
 * @return A pointer to the error wrapped by @c e is returned.
 */
static inline struct Error * error_wrapped_( const struct Error * e ) {
	if ( e == NULL ) {
		return NULL;
	} else {
		return ( struct Error * ) ( e->wrapped &~ ( Error_FREE | Error_MSG_FREE ) );
	}
}

/**
 * Obtains the wrapped error from an error.
 *
 * @param e Pointer to error.
 *
 * @return A pointer to the error wrapped by @c e is returned.
 */
static inline const struct Error * error_wrapped( const struct Error * e ) {
	return error_wrapped_( e );
}

/**
 * Determines whether the specified error must be freed on deletion.
 *
 * @param e Pointer to error.
 *
 * @return A non-zero value is returned if and only if the specified error must be freed on deletion.
 */
static inline int error_errorfree_( const struct Error * e ) {
	if ( e == NULL ) {
		return 0;
	}
	return ( e->wrapped & Error_FREE );
}

/**
 * Obtains the message of the specified error.
 *
 * @param e Pointer to error.
 *
 * @return A pointer to the error message of @c e is returned.
 */
static inline const char * error_msg( const struct Error * e ) {
	return e->msg;
}

/**
 * Determines whether the specified error must free its error message on deletion.
 *
 * @param e Pointer to error.
 *
 * @return A non-zero value is returned if and only if the message of the specified error must be freed on deletion.
 */
static inline int error_msgfree_( struct Error * e ) {
	if ( e == NULL ) {
		return 0;
	}
	return ( e->wrapped & Error_MSG_FREE );
}

/**
 * Changes allocation functions from the default malloc() and free().
 * Must be called before any other use of this library.
 *
 * @param f_alloc Allocation function with a signature like malloc().
 * @param f_free Freeing function with a signature like free().
 */
static inline void error_allocfuncs( void * ( *f_alloc )( size_t ), void ( *f_free )( void * ) ) {
	error_alloc_ = f_alloc;
	error_free_ = f_free;
}

/**
 * Creates a new error from a string.
 *
 * @param s Error string.
 *
 * @return A pointer to a newly allocated error with a copy of @c s as message is returned.
 */
static inline struct Error * error_new( const char * s ) {
	/* Sanity check */
	if ( s == NULL ) {
		return &Error_EMPTY_;
	}

	/* Build result FIXME: use error_newc() */
	struct Error * result = error_alloc_( sizeof( *result ) );
	if ( result == NULL ) {
		goto noerrmem;
	}
	result->msg = error_alloc_( Error_MAXLEN );
	if ( result->msg == NULL ) {
		goto nomsgmem;
	}
	strncpy( result->msg, s, Error_MAXLEN - 1 );
	result->msg[Error_MAXLEN - 1] = '\0';
	result->wrapped = Error_FREE | Error_MSG_FREE;

	return result;

nomsgmem:
	error_free_( result );
noerrmem:
	return &Error_NOMEM_;
}

/**
 * Creates a new error message from a string constant,
 * or a string that should not be freed.
 *
 * @param sc Immutable error string.
 *
 * @return A pointer to a newly allocated error with @c sc as message is returned.
 */
static inline struct Error * error_newc( const char * sc ) {
	/* Sanity check */
	if ( sc == NULL ) {
		return &Error_EMPTY_;
	}

	/* Build result */
	struct Error * result = error_alloc_( sizeof( *result ) );
	if ( result == NULL ) {
		goto noerrmem;
	}
	result->msg = ( char * ) sc;
	result->wrapped = Error_FREE;

	return result;

noerrmem:
	return &Error_NOMEM_;
}

/**
 * Creates a new formatted error message.
 *
 * @param format Format string (like for printf()).
 * @param ap Argument list.
 *
 * @return A pointer to a newly allocated error with formatted error message is returned.
 */
static inline struct Error * error_vnewf( const char * format, va_list ap ) {
	/* Sanity check */
	if ( format == NULL ) {
		return &Error_EMPTY_;
	}

	/* Build message */
	char * msg = error_alloc_( Error_MAXLEN );
	if ( msg == NULL ) {
		goto nomsg;
	}
	vsnprintf( msg, Error_MAXLEN, format, ap );
	struct Error * result = error_newc( msg );
	if ( result == &Error_NOMEM_ ) {
		goto noerr;
	}
	result->wrapped |= Error_MSG_FREE;

	return result;

noerr:
	error_free_( msg );
nomsg:
	return &Error_NOMEM_;
}

/**
 * Creates a new formatted error message.
 *
 * @param format Format string (like for printf()).
 * @param ... Arguments.
 *
 * @return A pointer to a newly allocated error with formatted error message is returned.
 */
static inline struct Error * error_newf( const char * format, ... ) Error_FORMAT_CHECK( 1, 2 );
static inline struct Error * error_newf( const char * format, ... ) {
	va_list ap;
	va_start( ap, format );
	struct Error * result = error_vnewf( format, ap );
	va_end( ap );

	return result;
}

/**
 * Destroys an error.
 *
 * @param e Pointer to error to destroy.
 */
static inline void error_del( struct Error * e ) {
	if ( e != NULL ) {
		error_del( error_wrapped_( e ) );
		if ( error_msgfree_( e ) ) {
			error_free_( e->msg );
		}
		if ( error_errorfree_( e ) ) {
			error_free_( e );
		}
	}
}

/**
 * Wraps an error message in another.
 *
 * @param e Pointer to error message.
 * 	Ownership is taken.
 * @param sc Immutable error string.
 *
 * @return A pointer to a newly allocated error with @c sc wrapped around @c e is returned.
 */
static inline struct Error * error_wrapc( struct Error * e, const char * sc ) {
	struct Error * result = error_newc( sc );
	if ( !error_errorfree_( result ) ) {
		/* Allocation failed */
		error_del( e );
	} else if ( e == NULL ) {
		/* Wrapped error empty */
		result->wrapped |= ( uintptr_t ) &Error_EMPTY_;
	} else {
		result->wrapped |= ( uintptr_t ) e;
	}

	return result;
}

/**
 * Wraps an error message in another.
 *
 * @param e Pointer to error message.
 * 	Ownership is taken.
 * @param s Error string.
 *
 * @return A pointer to a newly allocated error with a copy of @c s wrapped around @c e is returned.
 */
static inline struct Error * error_wrap( struct Error * e, const char * s ) {
	struct Error * result = error_new( s );
	if ( !error_errorfree_( result ) ) {
		/* Allocation failed */
		error_del( e );
	} else if ( e == NULL ) {
		/* Wrapped error empty */
		result->wrapped |= ( uintptr_t ) &Error_EMPTY_;
	} else {
		result->wrapped |= ( uintptr_t ) e;
	}

	return result;
}

/**
 * Wraps an error message in a formatted message.
 *
 * @param e Pointer to error message.
 * 	Ownership is taken.
 * @param format Format string.
 * @param ap Argument list.
 *
 * @return A pointer to a newly allocated error with a formatted string wrapped around @c e is returned.
 */
static inline struct Error * error_vwrapf( struct Error * e, const char * format, va_list ap ) {
	struct Error * result = error_vnewf( format, ap );
	if ( !error_errorfree_( result ) ) {
		/* Allocation failed */
		error_del( e );
	} else if ( e == NULL ) {
		/* Wrapped error empty */
		result->wrapped |= ( uintptr_t ) &Error_EMPTY_;
	} else {
		result->wrapped |= ( uintptr_t ) e;
	}

	return result;
}

/**
 * Wraps an error message in a formatted message.
 *
 * @param e Pointer to error message.
 * 	Ownership is taken.
 * @param format Format string.
 * @param ... Arguments.
 *
 * @return A pointer to a newly allocated error with a formatted string wrapped around @c e is returned.
 */
static inline struct Error * error_wrapf( struct Error * e, const char * format, ... ) Error_FORMAT_CHECK( 2, 3 );
static inline struct Error * error_wrapf( struct Error * e, const char * format, ... ) {
	va_list ap;
	va_start( ap, format );
	struct Error * result = error_vnewf( format, ap );
	va_end( ap );
	if ( !error_errorfree_( result ) ) {
		/* Allocation failed */
		error_del( e );
	} else if ( e == NULL ) {
		/* Wrapped error empty */
		result->wrapped |= ( uintptr_t ) &Error_EMPTY_;
	} else {
		result->wrapped |= ( uintptr_t ) e;
	}

	return result;
}

/**
 * Renders an error message through a rendering function.
 *
 * @param header String to be printed before message.
 * 	A null pointer is treated as an empty string.
 * @param e Pointer to error message.
 * @param trailer String to be printed after message.
 * 	A null pointer is treated as an empty string.
 * @param renderer Pointer to rendering function.
 * 	May be called multiple times.
 * 	A negative return value is considered to be an error,
 * 	causing an immediate return of this function.
 * @param ptr Pointer passed unaltered to rendering function.
 *
 * @return The last return value of @c renderer is returned.
 * 	A negative return value indicates an error.
 */
static inline int error_render( const char * header, const struct Error * e, const char * trailer, int ( *renderer )( void * ptr, const char * str ), void * ptr ) {
	/* Sanity checks */
	if ( e == NULL ) {
		return error_render( header, &Error_EMPTY_, trailer, renderer, ptr );
	}
	if ( renderer == NULL ) {
		return -1;
	}

	/* Render message */
	int rc;
	if ( header != NULL ) {
		rc = renderer( ptr, header );
		if ( rc < 0 ) {
			return rc;
		}
	}
	rc = renderer( ptr, e->msg );
	if ( rc < 0 ) {
		return rc;
	}
	const struct Error * wrapped = error_wrapped( e );
	if ( wrapped != NULL ) {
		rc = error_render( ": ", wrapped, NULL, renderer, ptr );
		if ( rc < 0 ) {
			return rc;
		}
	}
	if ( trailer != NULL ) {
		rc = renderer( ptr, trailer );
	}

	return rc;
}

/**
 * Wrapper around fputs().
 *
 * @param ptr File pointer.
 * @param s Pointer to string to be rendered.
 */
static int error_fputs_( void * ptr, const char * s ) {
	return fputs( s, ptr );
}

/**
 * Renders an error message to an output file.
 *
 * @param header String to be printed before message.
 * 	A null pointer is treated as an empty string.
 * @param e Pointer to error message.
 * @param trailer String to be printed after message.
 * 	A null pointer is treated as an empty string.
 * @param f Pointer to output file.
 *
 * @return On success, a nonnegative number is returned.\n
 * 	On error, a negative number is returned.
 */
static inline int error_frender( const char * header, const struct Error * e, const char * trailer, FILE * f ) {
	if ( f == NULL ) {
		return -1;
	}
	return error_render( header, e, trailer, error_fputs_, f );
}

#endif
