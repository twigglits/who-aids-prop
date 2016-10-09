#include "personimpl.h"
#include "person.h"
#include "jsonconfig.h"

PersonImpl::PersonImpl(Person &p) : m_person(p)
{
}

PersonImpl::~PersonImpl()
{
}

JSONConfig personImplJSONConfig(R"JSON(
        "PersonGeoDist": {
            "depends": null,
            "params": [
		[ "person.geo.dist2d", "distTypes2D" ]
            ],
            "info": [ 
                "TODO"
            ]
        })JSON");


