#ifndef _APP_MASTER_H_
#define _APP_MASTER_H_

/* rs485 management */
#define DEV_ALL          (0x00)        /* broadcast address */
#define DEV_BPL          (0x01)        /* based board fixed address */
#define DEV_CARD(x)      (0x80 + (x))    /* cards address is dynamically by position */


/* RO app reg (0x00-0x7F) */
#define DEV_S        (0x00)  //0 - 0x1F  device describe string
#define HWR          (0x20)  //hardware version
#define LFWR         (0x21)  //low firmware verison
#define HFWR         (0x22)  //high firmware version
#define CT           (0x23)  //this card type
#define UID_S        (0x24)  //0x24 - 0x2F  mcu uid
#define SLOT         (0x30)  //slot id
#define FAN_CNT      (0x31)
#define TEMP_CNT     (0x32)
#define CTRL_MODE    (0x33)
#define TEMP1        (0x34)  //temperature('C)
#define TEMP2        (0x35)
#define TEMP3        (0x36)
#define TEMP4        (0x37)
#define FAN_FG1_L    (0x38)  //fan speed monitor(rpm)
#define FAN_FG1_H    (0x39)
#define FAN_FG2_L    (0x3A)
#define FAN_FG2_H    (0x3B)
#define FAN_FG3_L    (0x3C)
#define FAN_FG3_H    (0x3D)
#define FAN_FG4_L    (0x3E)
#define FAN_FG4_H    (0x3F)

/* RW reg (0x80-0xFF) */
    #define RS485_REG_RWS      (0x40)  /* rs485 read/write reg offset start */
#define LED_MODE     (0x40)  //0_mcu_auto_ctl, 1_host_occupying
#define LED_COLOR    (0x41)  //0_off, 1_green, 2_red, 3_orange
#define LED_CYCLE    (0x42)  //led on/off cycle (100ms)
#define LED_DUTY     (0x43)  //led on time percentage
#define FAN_MODE     (0x4B)  //0_mcu_auto_ctl, 1_host_occupying
#define FAN_PWM1     (0x4C)  //fan speed control(percentage)
#define FAN_PWM2     (0x4D)
#define FAN_PWM3     (0x4E)
#define FAN_PWM4     (0x4F)
#define RWRES        (0x60)  //0x60-0x7F  host read/write ram area
    #define RS485_REG_RWE      (0x7F)  /* rs485 read/write reg offset end */

/* RW backend host, i2c/uart..etc */
    #define HOST_REG_RWS      (0x80)  /* host read/write reg offset start */
#define TRS          (0x80)  //token state
#define UPGRADE      (0x81)  //upgrade
    #define HOST_REG_RWE      (0xDF)  /* host read/write reg offset end */

/* i2c/rs485 exchange data */
    #define EXCANGE_REG_RWS      (0xE0)  /* host read/write reg offset start */
    #define EXCANGE_REG_RWE      (0xFF)  /* host read/write reg offset end */


/* TRS reg mask */
typedef enum trs_t {  //TRS value
    MASK_TRS_REQ    = 0x01,
}trs_t;

/* read          : src / dst / func / ret / reg / num / crc8 */
/* read_respond  : src / dst / func / ret / reg / num / dat[num] / crc8 */
/* write         : src / dst / func / ret / reg / num / dat[num] / crc8 */
/* write_respond : src / dst / func / ret / reg / num / crc8 */
/* read/write reg protocol */
#define P_SRC          (0)
#define P_DST          (1)
#define P_FUNC         (2)
#define P_RET          (3)
#define P_START        (4)
#define P_CNT          (5)
#define P_DAT          (6)
#define RW_BUF_LEN     (64)
#define RW_MIN_LEN     (P_DAT + 1)  //read/write cmd min len, crc in last 1 byte

/* func type */
typedef enum func_t {
    FREAD     =  0x30,
    FREAD_RE  =  0x31,
    FWRITE    =  0x32,
    FWRITE_RE =  0x33,
}func_t;

/* task tag : 0 is idle, reserve */
typedef enum app_task_type_t {
    TASK_SCAN_CT         = 1,  //扫描机箱所有插板类型
}app_task_type_t;

typedef struct board_info_t{
    char desc[32];
    int hw_ver;
    int sw_ver_sub;
    int sw_ver_main;
    int card_id;
    char card_name[32];
    int uid[3];
    int slot;
}board_info_t;

void app_init(void);

int set_gpio_value(int gpio, unsigned char value);
int gpio_release(int gpio);
int config_tty(int fd);
void close_all_file(void);

char rs485_read_cmd(unsigned char dev, unsigned char reg, unsigned char len, unsigned char *dat);
char rs485_write_cmd(unsigned char dev, unsigned char reg, unsigned char len, unsigned char *dat);

void sync_scan_slot(unsigned char xms, unsigned char *sta);

#endif
