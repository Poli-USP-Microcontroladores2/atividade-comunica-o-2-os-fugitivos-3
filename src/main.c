#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h> 
#include <string.h>
#include <zephyr/logging/log.h>

// Definições de Hardware e Logging
#define COM_PORT_NODE DT_CHOSEN(zephyr_shell_uart)
#define MAX_BUFFER_LEN 64

// Configuração do Logging
LOG_MODULE_REGISTER(com_sample, CONFIG_LOG_DEFAULT_LEVEL);

// Variáveis de I/O e Mensagens
K_MSGQ_DEFINE(input_msg_queue, MAX_BUFFER_LEN, 10, 4); 
static const struct device *const com_device = DEVICE_DT_GET(COM_PORT_NODE);
static char incoming_data_buffer[MAX_BUFFER_LEN];
static int incoming_pos;

// Sinalizador de Estado (Logicamente Controlado)
static volatile bool reception_active = false; 

// --- Função de Callback da UART (Interrupção RX) ---
void uart_interrupt_handler(const struct device *dev, void *user_data)
{
    uint8_t byte_received;
    
    // Verifica e limpa o estado da interrupção
    if (!uart_irq_update(com_device) || !uart_irq_rx_ready(com_device)) {
        return;
    }

    // Lê todos os bytes disponíveis na FIFO
    while (uart_fifo_read(com_device, &byte_received, 1) == 1) {

        // Só processa se o estado RX estiver ativo (reception_active == true)
        if (!reception_active) {
            continue; 
        }

        // Verifica o fim de linha (LF/CR)
        if ((byte_received == '\n' || byte_received == '\r') && incoming_pos > 0) {
            
            // Finaliza a string e enfileira
            incoming_data_buffer[incoming_pos] = '\0';
            k_msgq_put(&input_msg_queue, &incoming_data_buffer, K_NO_WAIT);
            incoming_pos = 0; // Prepara para a próxima mensagem
            
        } else if (incoming_pos < (sizeof(incoming_data_buffer) - 1)) {
            
            // Adiciona o byte ao buffer
            incoming_data_buffer[incoming_pos++] = byte_received;
        }
    }
}

// --- Função de Transmissão (Polling) ---
void send_message_poll(const char *data)
{
    int len = strlen(data);
    for (int i = 0; i < len; i++) {
        // Usa polling para transmissão síncrona dentro da thread TX
        uart_poll_out(com_device, data[i]); 
    }
}

// --- Thread de Transmissão Periódica (TX Burst) ---
void periodic_tx_and_state_toggle(void)
{
    char output_msg[MAX_BUFFER_LEN];
    int cycle_count = 0;
    const int burst_size = 3; 
    
    while (1) {
        k_sleep(K_SECONDS(5)); // Intervalo de 5 segundos

        // 1. Fase de Transmissão (TX)
        LOG_INF("Cycle %d: Initiating %d output messages", cycle_count, burst_size);
        for (int i = 0; i < burst_size; i++) {
            // Formato de mensagem ligeiramente diferente
            snprintk(output_msg, MAX_BUFFER_LEN, "DATA_PACKET %d.%d\r\n", cycle_count, i);
            send_message_poll(output_msg);
        }

        // 2. Fase de Toggle (Alternância RX)
        reception_active = !reception_active; 
        
        if (reception_active) {
            LOG_INF("RX functionality activated");
            incoming_pos = 0; // Limpa a posição
        } else {
            LOG_INF("RX functionality deactivated");
        }
        
        cycle_count++;
    }
}

// --- Thread Principal (RX Processing) ---
int main(void)
{
    char processed_input[MAX_BUFFER_LEN];

    if (!device_is_ready(com_device)) {
        LOG_ERR("Communication device not ready!");
        return 0;
    }

    // Configura o handler de interrupção
    uart_irq_callback_user_data_set(com_device, uart_interrupt_handler, NULL);

    // Habilita a interrupção de recepção (o controle lógico é feito pela variável 'reception_active')
    uart_irq_rx_enable(com_device);

    LOG_INF("System Ready - RX is currently passive");

    // Loop principal (Processing Loop)
    while (1) {
        // Espera indefinidamente por mensagens da fila RX
        if (k_msgq_get(&input_msg_queue, &processed_input, K_FOREVER) == 0) {
            // Loga a mensagem recebida em hexadecimal para evidência
            LOG_HEXDUMP_INF(processed_input, strlen(processed_input), "RECEIVED_DATA");
        }
    }
    // return 0; // Não alcançável
}

// Definição do Thread de TX/Toggle
K_THREAD_DEFINE(output_scheduler_thread, 1024, periodic_tx_and_state_toggle, NULL, NULL, NULL,
                7, 0, 0);