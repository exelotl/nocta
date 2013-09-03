#include "common.h"

nocta_unit* nocta_create_impl(nocta_unit base) {
	nocta_unit* unit = malloc(sizeof(nocta_unit));
	*unit = base;
	assert(unit->context);
	assert(unit->name);
	return unit;
}

void nocta_free(nocta_unit* unit) {
	if (unit->free) unit->free(unit); // call a custon free routine
	if (unit->data) free(unit->data); // free the custom data
	free(unit);
}

void nocta_process(nocta_unit* unit, int16_t* buffer, size_t length) {
	if (unit->process)
		unit->process(unit, buffer, length);
}


int nocta_get(nocta_unit* unit, int param_id) {
	if (param_id >= unit->num_params)
		return 0;
	nocta_param* param = &unit->params[param_id];
	return param->get(unit);
}

void nocta_set(nocta_unit* unit, int param_id, int val) {
	if (param_id >= unit->num_params)
		return;
	nocta_param* param = &unit->params[param_id];
	param->set(unit, val);
}

nocta_param* nocta_get_param(nocta_unit* unit, int param_id) {
	return param_id < unit->num_params ? &unit->params[param_id] : NULL;
}
