export MAKEROOT :=$(shell pwd)

TARGET_DIRS := eth_test \
				can_test \
				uart_test \
				key_test \
				storage_test \
				mtd_test \
				led_test \
				gpio_test \
				rtc_test \
				eeprom_test \
                adc_test \
                motor_test \
                stepper_test \
                pwm \
                spi_led \
				lcd_test \
				can_receive \
				rs485_test
				#drmmodeset\
				#audio_test\
				#camera_test \

include ./env.mk

ifeq ($(OPTION), mys-sam9x25_factory)
	SRCS := mys-sam9x25_factory.c
	TARGET := mys-sam9x25_factory_test
	TARGET_DIRS := eth_test \
				can_test \
				uart_test \
				key_test \
				storage_test \
				mtd_test \
				led_test \
				gpio_test \
				rtc_test \
				eeprom_test \
                adc_test \
                pwm \
				lcd_test \
				can_receive \
				rs485_test
endif
ifeq ($(OPTION),myd_a5d27_all)
	SRCS := myd_a5d27_all.c
	TARGET := myd_a5d27_all
	TARGET_DIRS := eth_test \
				can_test \
				uart_test \
				key_test \
				storage_test \
				mtd_test \
				led_test \
				gpio_test \
				rtc_test \
				eeprom_test \
                pwm \
				lcd_test \
				can_receive \
				drmmodeset \
				audio_test \
				rs485_test
endif
ifeq ($(OPTION), sam9x25v1_all)
	SRCS := sam9x25v1_all.c
	TARGET := sam9x25v1_all_test
endif
ifeq ($(OPTION), sama5d3x_factory)
	SRCS := sama5d3x_factory.c
	TARGET := sama5d3x_factory_test
endif
ifeq ($(OPTION), zhiyi_factory)
	SRCS := zhiyi_factory.c
	TARGET := zhiyi_factory_test
endif
ifeq ($(OPTION), odm-tap-factory)
	SRCS := odm-tap-factory.c
	TARGET := odm_tap_factory_test
endif

ifeq ($(OPTION), ACU3000_all)
	SRCS := ACU3000_all.c
	TARGET := ACU3000_all_test
	TARGET_DIRS := eth_test \
				can_test \
				uart_test \
				key_test \
				storage_test \
				mtd_test \
				led_test \
				gpio_test \
				rtc_test \
				eeprom_test \
                adc_test \
                motor_test \
                stepper_test \
				ACU3000_key \
                pwm \
                spi_led \
				lcd_test \
				can_receive \
				rs485_test
endif

ifeq ($(OPTION), IMEON_all)
	SRCS := IMEON_all.c
	TARGET := IMEON_all_test
	TARGET_DIRS += eth_test \
				can_test \
				uart_test \
				key_test \
				led_test \
				gpio_test \
				rtc_test \
                adc_test \
                pwm \
				lcd_test \
				can_receive \
				wifi_test \
				rs485_test
endif

#
# SAMA5D4
ifeq ($(OPTION), myd_ja5d4_all)
	SRCS := myd_ja5d4_all.c
	TARGET := myd_ja5d4_all_test
endif
ifeq ($(OPTION), am335x_factory)
	SRCS := am335x_factory.c
	TARGET := am335x_factory_test
endif

ifeq ($(OPTION), am335x_all_test)
	SRCS := am335x_all.c
	TARGET := am335x_all_test
endif
ifeq ($(OPTION), canada_sam9x5_factory)
	SRCS := canada_sam9x5_factory.c
	TARGET := canada_sam9x5_factory_test
endif

ifeq ($(OPTION), imx28_all_test)
	SRCS := imx28_all.c
	TARGET := imx28_all_test
endif

#
# AM437X
#
ifeq ($(OPTION), myd_c437x_all)
	SRCS := myd_c437x_all.c
	TARGET := myd_c437x_all
endif

ifeq ($(OPTION), ricoboard_all)
	SRCS := ricoboard_all.c
	TARGET := ricoboard_all_test
endif

ifeq ($(OPTION), ricoboard_dualeth)
	SRCS := ricoboard_dualeth.c
	TARGET := ricoboard_dualeth_test
endif

OBJS := $(patsubst %c, %o, $(SRCS))

.PHONY:all
all:$(OBJS)
#define build_obj
	for SubDir in $(TARGET_DIRS); do \
		if ! [ -d $$SubDir ]; then \
			echo "The $$SubDir is not exist!"; \
			exit 11; \
		fi; \
		echo "Building $$SubDir ..."; \
		make -C $$SubDir; \
		if [ $$? -ne 0 ]; then \
			echo "Building $$SubDir is failed!"; \
			exit 12; \
		fi; \
	done
#endef

	$(cal build_obj)
	${CC} ${CFLAGS} -o ./out/${TARGET} ./obj/* 
	cp scripts/auto-test.sh ./out/
	cp scripts/rtc_test.sh ./out/

clean:
	rm -rf ./obj/*

distclean:
	rm -rf ./obj/* ./out/*
