#include "es.h"
#include "term.h"
#include "version.h"

static const Term
	version_date_term = { VERSION_DATE, NULL },
	version_term = { VERSION, NULL };
static const List vdl = { (Term *) &version_date_term, NULL };
static const List versionstruct = { (Term *) &version_term, (List *)&vdl };
const List * const version = &versionstruct;
