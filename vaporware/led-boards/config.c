#include "config.h"
#include "stm_include/stm32/flash.h"

#ifdef TRACE_FLASH
	#include "usart.h"
#endif
#include "debug.h"

#define REPEAT(value,length) { [0 ... (length - 1)] = value }

/*
 * Configuration values in RAM for access by other modules.
 */

config_entry_t config = {
	.white_correction = REPEAT(0xffff, MODULE_LENGTH),
	.my_address = 0x00fd,
	.heat_limit = REPEAT(0xffff, HEAT_SENSOR_LEN),
	.physical_led = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
	.led_color = REPEAT(WHITE, MODULE_LENGTH)
};

/*
 * The configuration page. This is laid out in the following way:
 * There is a number of configuration slots, used one after the other
 * for storing a configuration. The state of each slot is kept in
 * entry_status, where 0xffff (the flash default value) stands for a free
 * slot, 0x5555 for the slot currently in use and 0x0000 for an old slot.
 * When all slots are used, the config page is erased and the process
 * starts again.
 *
 * The entry count is derived in the following way:
 * Page size: 1024B
 * Entry size with status word: sizeof(config_entry_t) + sizeof(uint16_t)
 * Entry count = Page size / Entry size
 */
#define ENTRY_COUNT (CONFIG_PAGE_SIZE / (sizeof(config_entry_t) + sizeof(uint16_t)))
typedef struct {
	uint16_t entry_status[ENTRY_COUNT];

	config_entry_t entries[ENTRY_COUNT];
} __attribute__ ((packed)) config_page_t;

config_page_t config_page __attribute__ ((section (".config"))) = {
	.entry_status = REPEAT(0xffff, ENTRY_COUNT),
	.entries = { [0 ... ENTRY_COUNT - 1] = {
			.white_correction = REPEAT(0xffff, MODULE_LENGTH),
			.my_address = 0xffff,
			.heat_limit = REPEAT(0xffff, HEAT_SENSOR_LEN),
			.physical_led = REPEAT(0xff, MODULE_LENGTH),
			.led_color = REPEAT(0xffffffff, MODULE_LENGTH)
		}
	}
};

/*
 * Loads the configuration stored in flash. If no configuration is found,
 * an E_NOCONFIG is returned.
 *
 * Returns an error/success code.
 */
error_t load_config() {
	// Look for an entry currently in use.
	int in_use = -1;
	for (int entry = 0; entry < ENTRY_COUNT; entry++) {
		if (config_page.entry_status[entry] == CONFIG_ENTRY_IN_USE) {
			in_use = entry;
			break;
		}
	}

#ifdef TRACE_FLASH
	debug_putchar((unsigned char) in_use);
#endif

	if (in_use == -1) {
		// No entry in use has been found.
		return E_NOCONFIG;
	}

	config = config_page.entries[in_use];

	return E_SUCCESS;
}

/*
 * Unlocks the flash memory for programming.
 */
static error_t flash_unlock() {
	if (FLASH_CR & FLASH_LOCK) {
		FLASH_KEYR = FLASH_KEY1;
		FLASH_KEYR = FLASH_KEY2;

		// Check if unlock was successful.
		if (FLASH_CR & FLASH_LOCK) {
			error(ER_BUG, STR_WITH_LEN("Flash unlocking failed"), EA_PANIC);
		}

		// Enable programming
		FLASH_CR |= FLASH_PG;
	}
	
	return E_SUCCESS;
}

/*
 * Locks the flash memory. Programming is no longer possible
 */
static error_t flash_lock() {
	FLASH_CR |= FLASH_LOCK;
	return E_SUCCESS;
}

/*
 * Checks the flash status for errors.
 */
static error_t flash_check_error() {
	// Wait for write to finish
	// Need to wait one more cycle, see erratum 2.7
	__asm("nop");
	while (FLASH_SR & FLASH_BSY);

	if (FLASH_SR & (FLASH_PGERR | FLASH_WRPRTERR)) {
		return E_FLASH_WRITE;
	} else {
		return E_SUCCESS;
	}
}

/*
 * Writes a word to flash and checks it afterwards.
 */
static error_t flash_write_check(uint16_t *address, uint16_t value) {
	error_t error;
	
	*address = value;
		
	error = flash_check_error();
	if (error != E_SUCCESS) return error;

	if (*address != value) return E_FLASH_WRITE;

	return E_SUCCESS;
}

/*
 * Erases the configuration page in flash.
 */
static error_t flash_erase_config_page() {
	FLASH_CR |= FLASH_PER;
	FLASH_AR = (uint32_t) &config_page;
	FLASH_CR |= FLASH_STRT;
	
	error_t error = flash_check_error();
	if (error) return error;

	// Verify
	uint16_t *start = (uint16_t*) &config_page;
	for (uint16_t *i = start;
	     i < start + CONFIG_PAGE_SIZE/sizeof(uint16_t);
	     i++) {
		if (*i != 0xffff) {
			return E_FLASH_WRITE;
		}
	}
	
	return E_SUCCESS;
}

/*
 * Saves the configuration to flash.
 */
error_t save_config() {
#ifdef TRACE_FLASH
	debug_string("save");
#endif
	
	error_t error;
	
	// Look for the entry last in use and an entry not yet used.
	// Assuming there is only one entry in use.
	int last_in_use = -1;
	int unused = -1;
	
	for (int entry = 0; entry < ENTRY_COUNT; entry++) {
		if (config_page.entry_status[entry] == CONFIG_ENTRY_IN_USE) {
			last_in_use = entry;
		}

		if (config_page.entry_status[entry] == CONFIG_ENTRY_EMPTY &&
		    unused == -1) {
			unused = entry;
		}
	}

#ifdef TRACE_FLASH
	debug_putchar((unsigned char) last_in_use);
	debug_putchar((unsigned char) unused);
#endif

	flash_unlock();

	// If no entries are free, erase config page and try again
	if (unused == -1) {
#ifdef TRACE_FLASH
		debug_string("erase");
#endif
		error = flash_erase_config_page();
		if (error != E_SUCCESS) return error;
		
		return save_config();
	}

	// Save the new configuration word by word.

	uint16_t *src = (uint16_t*) &config;
	uint16_t *dest = (uint16_t*) &config_page.entries[unused];

	_Static_assert(sizeof(config_entry_t) % sizeof(uint16_t) == 0, "config_entry_t must be repadded!");
	for (int i = 0; i < sizeof(config_entry_t) / sizeof(uint16_t); i++) {
		error = flash_write_check(dest + i, src[i]);
		if (error != E_SUCCESS) return error;
	}

#ifdef TRACE_FLASH
	debug_string("copy done");
#endif

	// The configuration was written successfully. Now update the status words.
	error = flash_write_check(config_page.entry_status + last_in_use, CONFIG_ENTRY_OLD);
	if (error != E_SUCCESS) return error;

	error = flash_write_check(config_page.entry_status + unused, CONFIG_ENTRY_IN_USE);
	if (error != E_SUCCESS) return error;

#ifdef TRACE_FLASH
	debug_string("status updated");
#endif

	flash_lock();

	return E_SUCCESS;
}

/*
 * Strings used in config_valid.
 */
#define CRLF "\r\n"

static const char *ADDRESS_IS_INVALID =
	"The board's address is invalid." CRLF;

static const char *ADDRESS_IS_BROADCAST =
	"Warning: The board's address is the broadcast address." CRLF;

static const char *LED_PERMUTATION_IS_INVALID =
	"The LED permutation is invalid." CRLF;

static const char *LED_COLOR_IS_INVALID =
	"The LED color is invalid." CRLF;

/*
 * Checks if the configuration in config is valid.  Returns 1 if the
 * configuration is valid, 0 otherwise.  This function may print an
 * explanatory message to the debug USART if the configuration is
 * found to be invalid.
 */
int config_valid() {
	int valid = 1;
	
	// Check if the module has been given a valid address.
	// Warn if the address is the broadcast address.
	if (config.my_address == 0xff) {
		debug_string(ADDRESS_IS_INVALID);
		valid = 0;
	}
	if (config.my_address == 0xfe) {
		debug_string(ADDRESS_IS_BROADCAST);
	}

	// Check that the LED permutation is really a valid permutation
	// from [0..MODULE_LENGTH] -> [0..MODULE_LENGTH]
	char led_seen[MODULE_LENGTH];
	for (int i = 0; i < MODULE_LENGTH; i++) {
		led_seen[i] = 0;
	}
	for (int i = 0; i < MODULE_LENGTH; i++) {
		led_seen[config.physical_led[i]]++;
	}
	for (int i = 0; i < MODULE_LENGTH; i++) {
		if (led_seen[i] != 1) {
			debug_string(LED_PERMUTATION_IS_INVALID);
			valid = 0;
		}
	}

	// Check that all colors are valid.
	for (int i = 0; i < MODULE_LENGTH; i++) {
		if (config.led_color[i] != RED &&
		    config.led_color[i] != GREEN &&
		    config.led_color[i] != BLUE &&
		    config.led_color[i] != WHITE) {
			debug_string(LED_COLOR_IS_INVALID);
			valid = 0;
		}
	}

	return valid;
}
