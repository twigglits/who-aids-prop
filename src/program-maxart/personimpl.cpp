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
		[ "person.geo.dist2d", "distTypes2D", [
				"discrete",
	    		        [
					[ "densfile", "${SIMPACT_DATA_DIR}SWZ10adjv4.tif" ],
				        [ "maskfile", "${SIMPACT_DATA_DIR}hhohho_mask.tiff" ],
					[ "width", 149.459 ],
					[ "height", 177.808 ],
					[ "flipy", "no" ]
				]
			]
		]
            ],
            "info": [ 
                "TODO"
            ]
        })JSON");


