#ifndef _GPIO_TEST
#define _GPIO_TEST

extern int gpio_export(const int gpio, const char *dir);
extern int gpio_set_value(const int gpio, const char value);

#endif
