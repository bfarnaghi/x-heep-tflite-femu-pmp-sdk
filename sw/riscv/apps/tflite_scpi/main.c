#include <stdio.h>
#include "models/lenet5_input.h"
#include "lenet5_test.h"
#include "scpi/scpi.h"
#include "uart.h"
#include "soc_ctrl.h"
#include "core_v_mini_mcu.h"
#include "mmio.h"
#include "tee_syscall.h"

#define ECHO 1

#define SCPI_IDN1 "MANUFACTURE"
#define SCPI_IDN2 "INSTR2013"
#define SCPI_IDN3 NULL
#define SCPI_IDN4 "01-02"

volatile soc_ctrl_t soc_ctrl;
volatile scpi_t scpi_context;
/* -------------------------------------------------------------- */
//__attribute__((section(".user_data")))
volatile uart_t uart;

__attribute__((section(".user_data")))
volatile int exit_scpi = 0;

__attribute__((section(".user_data")))
char buffer[2048];

/* linker symbols exported above */
extern uint8_t __user_start, __user_end;
extern uint8_t __user_stack_top;
extern uint8_t __user_text_start, __user_text_end;
extern uint8_t __user_data_start; 
/* -------------------------------------------------------------- */
void __attribute__((noinline)) SCPI_from_user(const char *buf, size_t len) {
  SCPI_Input(&scpi_context, buf, len);
  SCPI_Input(&scpi_context, "\r\n", 2);
  SCPI_Flush(&scpi_context);
}
/* -------------------------------------------------------------- */
scpi_result_t __attribute__((noinline)) InferExample(scpi_t * context) { 
  const char *out;
  size_t len;
  const int8_t *data = lenet_input_data;

  int a = infer((const char *) data, lenet_input_data_size, &out, &len);

  if (a == 0) {
    SCPI_ResultArrayInt8(context, (const int8_t *) out, len, SCPI_FORMAT_ASCII);
  } else {
    SCPI_ResultText(context, "Error");
  }
  return SCPI_RES_OK;
}

scpi_result_t __attribute__((noinline)) InferData(scpi_t * context) {
  const char *out;
  size_t out_len;

  const int8_t tflite_input_data[lenet_input_data_size];
  const int8_t *scpi_out;
  size_t scpi_len;
  SCPI_ParamArbitraryBlock(context, &scpi_out, &scpi_len, true);
  printf("Read: %d bytes\r\n", scpi_len);
  memcpy(tflite_input_data, scpi_out, scpi_len);

  int a = infer((const char *) tflite_input_data, lenet_input_data_size, &out, &out_len);
  if (a == 0) {
    SCPI_ResultArrayInt8(context, (const int8_t *) out, out_len, SCPI_FORMAT_ASCII);
  } else {
    SCPI_ResultText(context, "Inference error");
  }
  return SCPI_RES_OK;
}

scpi_result_t __attribute__((noinline))  Exit(scpi_t * context) {
    exit_scpi = 1;
    uart_write(&uart, (const uint8_t *) "Exiting...\r\n", 12);
    return SCPI_RES_OK;
}

volatile scpi_command_t scpi_commands[] = {
  { "NN:INFEr:EXAMple?", InferExample, 0},
  { "NN:INFEr:DATA?", InferData, 0},
  { "EXT", Exit, 0},
	SCPI_CMD_LIST_END
};

size_t __attribute__((noinline)) scrivi(scpi_t * context, const char * data, size_t len) {
    (void) context;
    size_t a = uart_write(&uart, (const uint8_t *) data, len);
    return a;
}

int __attribute__((noinline))  SCPI_Error(scpi_t * context, int_fast16_t err) {
    (void) context;
    uart_write(&uart, (const uint8_t *) "ERR!\r\n", 6);
    return 0;
}

scpi_result_t __attribute__((noinline)) SCPI_Control(scpi_t * context, scpi_ctrl_name_t ctrl, scpi_reg_val_t val) {
    return SCPI_RES_OK;
}
scpi_result_t __attribute__((noinline)) SCPI_Reset(scpi_t * context) {
    return SCPI_RES_OK;
}
scpi_result_t __attribute__((noinline))  SCPI_Flush(scpi_t * context) {
    return SCPI_RES_OK;
}

volatile scpi_interface_t scpi_interface = {
	.write = scrivi,
	.error = SCPI_Error,
	.control = NULL,
    .flush = NULL,
    .reset = NULL
};

#define SCPI_INPUT_BUFFER_LENGTH 2048
static char scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH];

#define SCPI_ERROR_QUEUE_SIZE 17
scpi_error_t scpi_error_queue_data[SCPI_ERROR_QUEUE_SIZE];

__attribute__((section(".user_data")))
static int modifier = 0;

__attribute__((section(".user_text"), aligned(4), noinline))
size_t __attribute__((noinline)) user_uart_loop(char *buf, size_t len) {
    
    while (!exit_scpi) {
        size_t i = 0;
        while (i < len - 1) {
            uint8_t c;
            c = tee_uart_getchar();
            if (c == '\\') {
                if (!modifier) modifier = 1;
                else {
                    buf[i] = c;
                    i++;
                    modifier = 0;
                }
                continue;
            }
            #if ECHO
            tee_uart_putchar(c);
            if (c == '\n') tee_uart_putchar('\r');
            else if (c == '\r') tee_uart_putchar('\n');
            #endif
            if (c == '\n' || c == '\r') {
                if (modifier) {
                    buf[i] = c;
                    i++;
                    modifier = 0;
                } else {
                    buf[i] = '\0';
                    break;
                }
            } else {
                buf[i] = c;
                i++;
                modifier = 0;
            }
        }

        if (i > 0) {
            tee_read_counters_arr();
            tee_infer(buf, i);
            tee_read_counters_arr();
            tee_read_counters_arr();
        }
    }
    while (1);
}

void switch_to_user_mode() {
    __asm__ volatile (
        "la   a0,  buffer      \n"   /* buf  = &buffer[0]  */
        "li   a1,  2048        \n"   /* len  = 2048        */
        
        "la t0, user_uart_loop     \n"  // Load user function address
        "csrw mepc, t0             \n"  // Set MEPC 

        "la sp, __user_stack_top   \n" 
        "li t2, -16                \n"  
        "add sp, sp, t2            \n"  
        
        /* Clear MPP, optionally set MPIE */
        "csrr t0,  mstatus\n"
        "li   t1, ~(3 << 11)\n"
        "and  t0, t0, t1\n"
        "csrw mstatus, t0\n"

        "mret                      \n"  
        
        :
        :
        : "t0", "t1", "memory"
    );
}

void __attribute__((noinline)) uart_scpi(scpi_t * context, uart_t * uart) {
  printf("Starting SCPI loop...\r\n");
  switch_to_user_mode();
}

mmio_region_t mmio_region_from_adr(uintptr_t address) {
  return (mmio_region_t){
      .base = (volatile void *)address,
  };
}

void pmp_setup(void)
{
    __asm__ volatile(
        "csrw  pmpcfg0, zero\n\t"          /* disable entry 0 first      */
        "li    t0,   0x1e000\n\t"          /* 0x00078000 >> 2 → bottom   */
        "csrw  pmpaddr0, t0\n\t"
        "li    t0,   0x20000\n\t"          /* 0x00080000 >> 2 → top      */
        "csrw  pmpaddr1, t0\n\t"           /* entry 1 addr               */
        "li    t0,   0x0f\n\t"             /* TOR | R | W | X | L        */
        "slli t0, t0, 8\n\t"
        "csrs pmpcfg0, t0\n\t"             /* program **entry 1** cfg    */
        "fence.i\n\t"
    );
}

int main() {
  printf("Initializing peripherals...\r\n");
  soc_ctrl.base_addr = mmio_region_from_adr((uintptr_t)SOC_CTRL_START_ADDRESS);
  printf("Initialized soc_ctrl\r\n");
  printf("soc_ctrl.base_addr: %p\r\n", soc_ctrl.base_addr);
    uart.base_addr   = mmio_region_from_adr((uintptr_t)UART_START_ADDRESS);
    uart.baudrate    = 115200;
    uart.clk_freq_hz = soc_ctrl_get_frequency(&soc_ctrl);
    if (uart_init(&uart) != kErrorOk) {
        return;
    }
  printf("Initialized UART\r\n");
  printf("uart.base_addr: %p\r\n", uart.base_addr);
  printf("uart.baudrate: %d\r\n", uart.baudrate);
  printf("uart.clk_freq_hz: %d\r\n", uart.clk_freq_hz);

    SCPI_Init(&scpi_context, 
              scpi_commands, 
              &scpi_interface, 
              scpi_units_def, 
              SCPI_IDN1, SCPI_IDN2, SCPI_IDN3, SCPI_IDN4, 
              scpi_input_buffer, SCPI_INPUT_BUFFER_LENGTH,
              scpi_error_queue_data, SCPI_ERROR_QUEUE_SIZE);

  printf("Initialized x.ruSCPI\r\n");
  // Print available SCPI commands
  printf("Available SCPI commands:\r\n");
  for (int i = 0; scpi_commands[i].pattern != NULL; i++) {
    printf("%s\r\n", scpi_commands[i].pattern);
  }
  init_tflite();
  printf("Initialized TFLite\r\n");

  pmp_setup();

  uart_scpi(&scpi_context, &uart);

  return 0;
}

void __attribute__((optimize("O0"))) should_not_happen() {
    printf("This should not happen\r\n");
}