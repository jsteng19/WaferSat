#ifndef __PADC_H__
#define __PADC_H__

#include "ch.h"
#include "hal.h"
#include "common.h"

// We need a struct because different calls to get_vbat/temp are different 
// readings

struct adc_t {
	uint16_t vbat;
	uint16_t therm;
	enum SensorErr err;
};
#define adc_t_init() ((struct adc_t){0, 0, SENSOR_OK})
#define adc_t_fields(dptr) ((dptr)->vbat), ((dptr)->therm), ((dptr)->err)
#define ADC_CSV_STR "%u,%u,%u"
#define ADC_HUMAN_STR "vbat:%u therm:%u err:%u"

enum SensorErr adc_init(void);
enum SensorErr adc_shutdown(void);
struct adc_t adc_get(void);

#endif /* __ADC_H__ */

