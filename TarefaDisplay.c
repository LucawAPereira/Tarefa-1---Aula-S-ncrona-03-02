#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"


#define IS_RGBW false
#define NUM_PIXELS 25
#define WS2812_PIN 7
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define BotaoA 5
#define BotaoB 6
#define ledgreen 11
#define ledblue 12

uint8_t led_r = 0; // Intensidade do vermelho
uint8_t led_g = 0; // Intensidade do verde   // variaveis ws2812
uint8_t led_b = 20; // Intensidade do azul
uint8_t var=0;

void gpio_irq_handler(uint gpio, uint32_t events);

// ws2812
bool led_buffer0[NUM_PIXELS] = { // numero 0
  0, 1, 1, 1, 0, 
  0, 1, 0, 1, 0,                      // [0,1,2,3,4,
  0, 1, 0, 1, 0,                      //  5,6,7,8,9
  0, 1, 0, 1, 0,                      //  10,11,12,13,14   ---> como realmente é a ordem da matriz de leds
  0, 1, 1, 1, 0                       //  15,16,17,18,19
};                                      //  20,21,22,23,24]

bool led_buffer1[NUM_PIXELS] = { // numero 1
  0, 0, 1, 0, 0, 
  0, 0, 1, 0, 0,                      
  0, 0, 1, 0, 0,                      
  0, 1, 1, 0, 0,                      
  0, 0, 1, 0, 0                       
}; 

bool led_buffer2[NUM_PIXELS] = { // numero 2
  0, 1, 1, 1, 0, 
  0, 1, 0, 0, 0,                      
  0, 1, 1, 1, 0,                      
  0, 0, 0, 1, 0,                      
  0, 1, 1, 1, 0                       
}; 

bool led_buffer3[NUM_PIXELS] = {// numero 3
  0, 1, 1, 1, 0, 
  0, 0, 0, 1, 0,                      
  0, 1, 1, 1, 0,                      
  0, 0, 0, 1, 0,                      
  0, 1, 1, 1, 0                       
}; 

bool led_buffer4[NUM_PIXELS] = { // numero 4
  0, 1, 0, 0, 0, 
  0, 0, 0, 1, 0,                      
  0, 1, 1, 1, 0,                      
  0, 1, 0, 1, 0,                      
  0, 1, 0, 1, 0                       
}; 

bool led_buffer5[NUM_PIXELS] = {// numero 5
  0, 1, 1, 1, 0, 
  0, 0, 0, 1, 0,                      
  0, 1, 1, 1, 0,                      
  0, 1, 0, 0, 0,                      
  0, 1, 1, 1, 0                       
}; 

bool led_buffer6[NUM_PIXELS] = {// numero 6
  0, 1, 1, 1, 0, 
  0, 1, 0, 1, 0,                      
  0, 1, 1, 1, 0,                      
  0, 1, 0, 0, 0,                      
  0, 1, 1, 1, 0                       
}; 

bool led_buffer7[NUM_PIXELS] = {// numero 7
  0, 1, 0, 0, 0,                      // [0,1,2,3,4,
  0, 0, 0, 1, 0,                      //  5,6,7,8,9
  0, 1, 0, 0, 0,                      //  10,11,12,13,14   ---> como realmente é a ordem da matriz de leds
  0, 0, 0, 1, 0,                      //  15,16,17,18,19
  0, 1, 1, 1, 0                       //  20,21,22,23,24]
};                                      

bool led_buffer8[NUM_PIXELS] = {// numero 8
  0, 1, 1, 1, 0, 
  0, 1, 0, 1, 0,                      
  0, 1, 1, 1, 0,                      
  0, 1, 0, 1, 0,                      
  0, 1, 1, 1, 0                       
}; 

bool led_buffer9[NUM_PIXELS] = {// numero 9
  0, 1, 1, 1, 0, 
  0, 0, 0, 1, 0,                      
  0, 1, 1, 1, 0,                      
  0, 1, 0, 1, 0,                      
  0, 1, 1, 1, 0                       
}; 


static inline void put_pixel(uint32_t pixel_grb)  // protocolo WS18
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) //  protoclo WS2812
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}


void set_one_led(uint8_t r, uint8_t g, uint8_t b, int buffer_index)  // funcao para escrever o numero na matriz de leds
{
    bool* led_buffer[] = {led_buffer0, led_buffer1, led_buffer2, led_buffer3, led_buffer4, led_buffer5, led_buffer6, led_buffer7 ,led_buffer8 ,led_buffer9}; // vetor desenho numeros

    // Define a cor com base nos parâmetros fornecidos
    uint32_t color = urgb_u32(r, g, b);
    bool* buffer = led_buffer[buffer_index]; // armazena o array na variavel buffer, que será enviado para o vetor de 25 leds correspondente ao desenho 

    // Define todos os LEDs com a cor especificada
    
        for (int i = 0; i < NUM_PIXELS; i++)
        {
        if (buffer[i])
        {
        put_pixel(color); // Liga o LED
        }
        else
        {
        put_pixel(0);  // Desliga o LED
        }
    }

}

// ws2812

static volatile uint32_t last_time = 0; // Armazena o tempo do último evento (em microssegundos)


int main()
{
  stdio_init_all(); 
  PIO pio = pio0;
  int sm = 0;                 
  uint offset = pio_add_program(pio, &ws2812_program);

  ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);      // ws2812

  // I2C Initialisation. Using it at 400Khz.
  i2c_init(I2C_PORT, 400 * 1000);

  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
  gpio_pull_up(I2C_SDA); // Pull up the data line
  gpio_pull_up(I2C_SCL); // Pull up the clock line
  ssd1306_t ssd; // Inicializa a estrutura do display
  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
  ssd1306_config(&ssd); // Configura o display
  ssd1306_send_data(&ssd); // Envia os dados para o display

  // Limpa o display. O display inicia com todos os pixels apagados.
  ssd1306_fill(&ssd, false);
  ssd1306_send_data(&ssd);

  bool cor = true;

  gpio_init(ledgreen);
  gpio_set_dir(ledgreen, GPIO_OUT);
  gpio_put(ledgreen, 0);

  gpio_init(ledblue);
  gpio_set_dir(ledblue, GPIO_OUT);
  gpio_put(ledblue, 0);

  gpio_init(BotaoA);
  gpio_set_dir(BotaoA, GPIO_IN);
  gpio_pull_up(BotaoA);

  gpio_init(BotaoB);
  gpio_set_dir(BotaoB, GPIO_IN);
  gpio_pull_up(BotaoB);

  gpio_set_irq_enabled_with_callback(BotaoA, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
  gpio_set_irq_enabled_with_callback(BotaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

  while (true)
{
    // Aguarda um caractere via comunicação serial
    int caractere = getchar();

    // Verifica se recebeu um caractere válido
    if (caractere != EOF)
    {
        // Limpa o display
        ssd1306_fill(&ssd, false);
        
        // Converte o caractere para string
        char texto[2] = {caractere, '\0'};

        // Verifica se o caractere é um número ou letra
        if (caractere >= '0' && caractere <= '9') {
            // Se for número, converte para índice e exibe na matriz WS2812
            var = caractere - '0'; // Converte o ASCII '0' que tem seu valor igual a 48 e subtrai de caractere que sera o numero enviado, o "1" em ASCII é igual a 49, "2" = 50 e assim por diante
            set_one_led(led_r, led_g, led_b, var); 
            char texto[2]; // Vetor para armazenar o caractere, necessario para desenhar para dentro do display
            sprintf(texto, "%d", var); // Converte o número para string
            ssd1306_draw_string(&ssd, texto, 50, 25);  // Exibe o número no display
        } else {
            ssd1306_draw_string(&ssd, texto, 50, 25);  // Exibe a letra no display
        }

        ssd1306_send_data(&ssd);  // Atualiza o display
        
        sleep_ms(100);
    }
}
}


void gpio_irq_handler(uint gpio, uint32_t events)
{
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    if (current_time - last_time > 350000) // 350 ms de debounce
    {
        last_time = current_time; // Atualiza o tempo do último evento

        gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
        gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
        gpio_pull_up(I2C_SDA); // Pull up the data line
        gpio_pull_up(I2C_SCL); // Pull up the clock line
        ssd1306_t ssd; // Inicializa a estrutura do display
        ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
        ssd1306_config(&ssd); // Configura o display
        ssd1306_send_data(&ssd);
      
        if (gpio == BotaoA) {
            // Verifica o estado atual do LED verde
            if (gpio_get(ledgreen)) { 
                ssd1306_draw_string(&ssd, "Led Verde apagado", 50, 2);
            } else {  
                ssd1306_draw_string(&ssd, "Led Verde aceso", 50, 2);
            }
            gpio_put(ledgreen, !gpio_get(ledgreen));  // Alterna o estado do LED verde
            ssd1306_send_data(&ssd);  // Atualiza o display
        } 
        if (gpio == BotaoB) {
            // Verifica o estado atual do LED azul
            if (gpio_get(ledblue)) { 
                ssd1306_draw_string(&ssd, "Led Azul apagado", 50, 2);
            } else {  
                ssd1306_draw_string(&ssd, "Led Azul aceso", 50, 2);
            }
            gpio_put(ledblue, !gpio_get(ledblue));  // Alterna o estado do LED azul
            ssd1306_send_data(&ssd);  // Atualiza o display
        }
    }
}