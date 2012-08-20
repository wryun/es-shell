/* version.c -- version number ($Revision: 1.6 $) */
#include "es.h"
static const char id[] = "@(#)es version 0.8: 20 Mar 1993";
const char * const version = &id[sizeof "@(#)" - 1];
