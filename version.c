#include "es.h"
#include "term.h"
#include "version.h"

static const Term
	version_term = { VERSION, NULL };
static const List versionstruct = { (Term *) &version_term, NULL };
const List * const version = &versionstruct;
