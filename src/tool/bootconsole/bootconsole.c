#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <fcntl.h>
#include <termios.h>
#include <getopt.h>

#include <bluetooth.h>
#include <hci.h>
#include <hci_lib.h>

#include <glib.h>
#include <lib/uuid.h>
#include <lib/sdp.h>

#include <attrib/att.h> 
#include <attrib/gattrib.h>
#include <btio/btio.h>
#include <src/shared/util.h>

#include <attrib/gatttool.h>

#include "gatt.h"

#define TX_MTU          	20
#define FLASH_COPY_MAXLEN       256
#define RESP_OK			'O'
#define RESP_ERR		'E'

#define UPDATE_DPCT_UPLOAD	0
#define UPDATE_DPCT_DOWNLOAD	1
#define UPDATE_DPCT_COPY	2
#define UPDATE_DPCT_EXTUPLOAD	3


static GMainLoop 		*g_event_loop;
static volatile gboolean 	g_ble_write_done = 1;

enum BF_State {
    BF_START,
    BF_SETADDR,
    BF_SETADDR_WAIT_OK,
    BF_WRITE_DATA,
    BF_WRITE_WAIT_OK,
    BF_READ_DATA,
    BF_READ_WAIT_OK,
    BF_JMP_WAIT_OK,
    BF_FLASH_COPY,
    BF_FLASH_COPY_WAIT_OK,
    BF_EXT_WRITE_BUF,
    BF_EXT_WRITE_SPI,
    BF_EXT_WRITE_SPI_WAIT_OK,
    BF_DONE_WAIT_OK,
    BF_EXT_DONE,
};

static enum BF_State 		g_op_state;
static int 			g_uart_mode = 1;
static int 			g_flash_copy_mode = 0;
static int 			g_ext_write_mode = 0;

static uint16_t 		g_ble_txhnd = 0x11;
static uint16_t 		g_ble_exthnd = 0x12;
static const char 		*g_ble_mac = "20:C3:8F:EF:5F:06";
static GAttrib 			*g_ble_attrib;
static int 			g_ble_mac_flag = 0;

static const char 		*g_uart_fname = "/dev/ttyUSB0";
static int			g_uart_fname_flag = 0;

static const char		*g_input_fname = 0;
static const char 		*g_output_fname = 0;

static int 			g_output_len = 0;
static int 			g_input_len;

static int 			g_uart_fd;
static int 			g_input_fd;
static int 			g_output_fd;

static int 			g_offset = 0;

static int 			g_reverse_input = 0;

static unsigned int 		g_mem_addr = 0x10000;
static unsigned int 		g_jmp_addr = 0x10000;

static unsigned int 		g_flash_memsrc_addr = 0x10000;
static unsigned int 		g_flash_dst_addr;
static unsigned int 		g_flash_copy_len;

static unsigned int 		g_flash_copy_addr_set;
static int 			g_jmp_set = 0;

static int 			g_done_pct = 0;

static guint 			g_timeout_id = 0;
static int 			g_ble_disconnected = 0;

static unsigned char		g_ext_checksum = 0;


typedef struct restart_checkpoint {
    int 		valid;    
    enum BF_State	state;

    int 		offset;

    unsigned int 	flash_dst_addr;
    unsigned int 	flash_memsrc_addr;
    unsigned int 	flash_copy_count;
    unsigned int 	flash_copy_addr_set;

} restart_checkpoint_t;

restart_checkpoint_t		g_checkpoint;



static enum BF_State run_state_machine(
				enum BF_State state, 
				const char *input, int len);



static void 		init_ble_reader();
static void 		init_terminal_reader();
static void 		init_uart_reader();
static gboolean		ble_listen_start(gpointer user_data);;

static gboolean		restart_operations(gpointer user_data);

/****************************************************************/

static void
delete_restart_checkpoint()
{
    g_checkpoint.valid = 0;
}


static void
create_restart_checkpoint(enum BF_State state)
{
    g_checkpoint.valid = 1;
    g_checkpoint.state = state;
    g_checkpoint.flash_dst_addr = g_flash_dst_addr;
    g_checkpoint.flash_memsrc_addr = g_flash_memsrc_addr;
    g_checkpoint.flash_copy_addr_set = g_flash_copy_addr_set;
    g_checkpoint.offset = g_offset;
}


static void
restore_restart_checkpoint()
{
    if (g_checkpoint.valid != 1)
	errx(1, "Logic error, invalid checkpoint");

    g_op_state = g_checkpoint.state;
    g_flash_dst_addr = g_checkpoint.flash_dst_addr;
    g_flash_memsrc_addr = g_checkpoint.flash_memsrc_addr;
    g_flash_copy_addr_set = g_checkpoint.flash_copy_addr_set;
    g_offset = g_checkpoint.offset;
}

/****************************************************************/

static unsigned int
str_to_uint(const char *str)
{
    char *end;
    unsigned int i;

    i = strtoul(str, &end, 0);
    if (*end != 0)
	errx(1, "Error converting string to integer: %s", str);
    return i;
}


static void
check_port_set()
{

    if (g_uart_mode == 1) {
	if (g_uart_fname_flag == 0)
		errx(1, "Please set the UART device path using the -D option");
    } else if (g_ble_mac_flag == 0) {
	errx(1, "Please provide A BLE mac address -b option");
    }
}


static void
parse_args(int argc, char **argv)
{
    static const struct option s_lopts[] = {
				{ "flash-app", 1, 0, 0 },
				{ "run-app", 1, 0, 0 },
				{ "flash-rbf", 1, 0, 0 },
				{ "flash-fs", 1, 0, 0 },
				{ "sio-terminal", 0, 0, 0 },
				{ 0, 0, 0, 0 },
			    };
    static const char *s_opts = "Bb:h:H:f:D:m:j:M:J:ro:c:X";
    const char *larg;
    int opt, lopts_index;

    while ( (opt = getopt_long(
		    	argc, argv, s_opts,
			s_lopts, &lopts_index)) != -1) {
	switch (opt) {
	case 0: 
	    check_port_set();
	    switch (lopts_index) {
	    case 0: /* flash app */
		g_mem_addr = 0x10000;
		g_flash_memsrc_addr = 0x10000;
		g_flash_dst_addr = 0x80000;
		g_flash_copy_mode = 1;
		g_input_fname = strdup(optarg);
		break;
	    case 1: /* run app */
		g_mem_addr = 0x10000;
		g_jmp_addr = 0x10000;
		g_input_fname = strdup(optarg);
		g_jmp_set = 1;
		break;
	    case 2: /* flash firmware */
		g_mem_addr = 0x10000;
		g_flash_memsrc_addr = 0x10000;
		g_flash_dst_addr = 0x00000;
		g_flash_copy_mode = 1;
		g_input_fname = strdup(optarg);
		g_reverse_input = 1;
		break;
	    case 3: /* flash filesystem */
		g_mem_addr = 0x10000;
		g_flash_memsrc_addr = 0x10000;
		g_flash_dst_addr = 0x180000;
		g_flash_copy_mode = 1;
		g_input_fname = strdup(optarg);
		break;
	    case 4: /* sio terminal */
		break;
	    default:
		/* EDOOFUS */
		break;
	    }
	    return;
	case 'X':
	    g_ext_write_mode = 1;
	    break;
	case 'r':
	    g_reverse_input = 1;
	    break;
	case 'j':
	    g_jmp_addr = str_to_uint(optarg) & (~0x11);
	    g_jmp_set = 1;
	    break;
	case 'm':
	    g_mem_addr = str_to_uint(optarg) & (~0xf);
	    break;
	case 'J':
	    g_flash_dst_addr = str_to_uint(optarg) & (~0xff);
	    g_flash_copy_mode = 1;
	    break;
	case 'M':
	    g_flash_memsrc_addr = str_to_uint(optarg) & (~0xff);
	    break;
	case 'B':
	    g_uart_mode = 0;
	    break;
	case 'b':
	    g_ble_mac = strdup(optarg);
	    g_ble_mac_flag = 1;
	    break;
	case 'h':
	    g_ble_txhnd = str_to_uint(optarg);
	    break;
	case 'H':
	    g_ble_exthnd = str_to_uint(optarg);
	    break;
	case 'f':
	    g_input_fname = strdup(optarg);
	    break;
	case 'D':
	    g_uart_fname = strdup(optarg);
	    g_uart_fname_flag = 1;
	    break;
	case 'o':
	    g_output_fname = strdup(optarg);
	    break;
	case 'c':
	    g_output_len = str_to_uint(optarg);
	    break;
	default:
            errx(1,
		"Usage: %s [-B] [-X] [-r] [-b mac] [-h hnd] [-H ext_hnd]"
		"[-D uart ] "
		"[-f input_file] "
		"[-j jmp_addr] "
		"[-m mem_addr] "
		"[-M flash_memsrc_addr] "
		"[-J flash_dst_addr] "
		"[-o output_file] "
		"\n"
		"Parameters:\n"
		"\n"
		"-B : BLE mode, UART mode if not specified\n"
		"-X : BLE extension mode\n"
		"-r : Reverse input data, required for RBF input files\n"
		"-b <mac> : Board BLE address (hcitool lescan)\n"
		"-h <hnd> : GATT TX Handle (0x25)\n"
		"-H <hnd> : GATT Extension Handle (0x28)\n"
		"-D <uart> : UART device (/dev/ttyUSB0)\n"
		"-f <file> : Input Data-file, terminal mode if not specified\n"
		"-j <addr> : Jump-to address post input file upload\n"
		"-m <addr> : File upload address\n"
		"-M <addr> : Flash copy, memory start address\n"
		"-J <addr> : Flash copy, flash destination address\n"
		"-o <file> : Output (read memory mode) Data-file\n"
		"-c <len>  : Length of memory to be read\n"
		"\n", argv[0]);
	}
    }
}


static unsigned char
get_bit(char c, int i)
{
    unsigned char v = (1 << i);
    if (c & v)
	return v;
    return 0;
}


static void
reverse_bits(gchar *buf, int len)
{
    if (len > 0 && g_reverse_input) {
	for (int i = 0; i < len; ++i) {
	    buf[i] = 
		(get_bit(buf[i], 0) << 7) |
		(get_bit(buf[i], 1) << 5) |
		(get_bit(buf[i], 2) << 3) |
		(get_bit(buf[i], 3) << 1) |
		(get_bit(buf[i], 4) >> 1) |
		(get_bit(buf[i], 5) >> 3) |
		(get_bit(buf[i], 6) >> 5) |
		(get_bit(buf[i], 7) >> 7);
	}	
    }
}


static gchar
compute_checksum(const gchar *buf, int len)
{
    gchar x = buf[0];
    for (int i = 1; i < len; ++i)
	x ^= buf[i]; 
    return x;
}


/****************************************************************/

static void
disable_response_timer()
{
    if (g_timeout_id != 0)
    	g_source_remove(g_timeout_id);
    g_timeout_id = 0;
}


static gboolean
response_timeout_callback(gpointer act)
{
    g_timeout_add(1, restart_operations, 0);
    return G_SOURCE_REMOVE;
}


static void
enable_response_timer()
{
    if (g_timeout_id != 0) {
	warnx("Enable response timer: Timer already active / disabling");
    	disable_response_timer();
    }

    g_timeout_id = g_timeout_add(2000, response_timeout_callback, 0);
}


inline static int
is_terminal_mode()
{
    return ((g_input_fname == 0) && (g_output_fname == 0) ? 1 : 0);
}


inline static void
set_terminal_mode()
{
    g_input_fname = 0;
    g_output_fname = 0;
}


inline static int
is_read_mode()
{
    return (g_output_fname == 0 ? 0 : 1);
}


static int
is_uart_mode()
{
    return g_uart_mode;
}

static int
is_ext_write_mode()
{
    return g_ext_write_mode;
}

/****************************************************************/

static void
ble_events_handler(const uint8_t *pdu, uint16_t len, gpointer user_data)
{
    // GAttrib *attrib = user_data;
    // uint16_t handle;

    disable_response_timer();

    //handle = get_le16(&pdu[1]);

    switch (pdu[0]) {
    case ATT_OP_HANDLE_NOTIFY:
	/* Incoming data */
	if (is_terminal_mode()) {
	    write(1, pdu + 3, len - 3);
	} else {
	    g_op_state = run_state_machine(g_op_state, pdu + 3, len - 3);
	}
	break;
    case ATT_OP_HANDLE_IND:
	/* Unexpected, should not happen */
	errx(1, "Unexpected BLE Indication");
    default:
	errx(1, "Unexpected BLE opcode");
    }

    return;
}


static GAttrib *s_lastreq_attrib;
static uint16_t s_lastreq_handle;
static gint8 s_lastreq_data[TX_MTU];
static gint8 s_lastreq_len;
static gint8 s_lastreq_retries;

#define TX_MAX_RETRIES		16
#define TX_RETRY_WAIT_USEC	100000

static void
gatt_request_cb(guint8 status, const guint8 *pdu, 
	  	guint16 len, gpointer user_data)
{ 
    if (status == 0)
	g_ble_write_done = 1;
    else {
	usleep(TX_RETRY_WAIT_USEC);
	gatt_write_char(s_lastreq_attrib, s_lastreq_handle,
			s_lastreq_data, s_lastreq_len,
			gatt_request_cb, NULL);
    }

    if(++s_lastreq_retries > TX_MAX_RETRIES)
	errx(1, "Terminal mode, transmit error, too many retries.");
}


static guint
gatt_request(GAttrib *attrib, uint16_t handle, const uint8_t *value,
          int vlen, GAttribResultFunc notify, gpointer user_data)
{
    g_ble_write_done = 0;
    s_lastreq_attrib = attrib;
    s_lastreq_handle = handle;
    memcpy(s_lastreq_data, value, vlen);
    s_lastreq_len = vlen;
    s_lastreq_retries = 0;
    return gatt_write_char(attrib, handle, value, vlen, gatt_request_cb, NULL);
}


static void update_done_pct(int mode);

static void
gatt_write_cb(gpointer data)
{
    g_ble_write_done = 1;

    if (is_terminal_mode())
	return;

    if (g_op_state == BF_EXT_DONE) {
	sleep(3);
	exit(0);
    }

    if (g_op_state == BF_START) {
	disable_response_timer();
	warnx("Disabling FPGA via CONF and CE pins");
	g_op_state = run_state_machine(BF_EXT_WRITE_BUF, 0, 0);
    } else if (g_op_state == BF_EXT_WRITE_BUF) {
	disable_response_timer();
	g_offset += 16;
	update_done_pct(UPDATE_DPCT_EXTUPLOAD);
	if ((g_offset & 0xff) == 0) {
	    g_op_state = run_state_machine(BF_EXT_WRITE_SPI, 0, 0);
	} else {
	    g_op_state = run_state_machine(BF_EXT_WRITE_BUF, 0, 0);
	}
    } 
}


static guint
gatt_write(GAttrib *attrib, uint16_t handle, const uint8_t *value,
          int vlen, GDestroyNotify notify, gpointer user_data)
{
    g_ble_write_done = 0;
    return gatt_write_cmd(attrib, handle, value, vlen, gatt_write_cb, NULL);
}


static gboolean
ble_listen_start(gpointer user_data)
{
    GAttrib *attrib = user_data;
    g_attrib_register(
        attrib, ATT_OP_HANDLE_NOTIFY, GATTRIB_ALL_HANDLES,
        ble_events_handler, attrib, NULL);
    g_attrib_register(
        attrib, ATT_OP_HANDLE_IND, GATTRIB_ALL_HANDLES,
        ble_events_handler, attrib, NULL);
    return FALSE;
}


static void
ble_connect_cb(GIOChannel *chan, GError *err, gpointer user_data)
{
    if (err) {
        warnx("%s", err->message);
        g_error_free(err);
    	g_io_channel_unref (chan);
	init_ble_reader();
        return;
    }

    uint16_t mtu, cid;
    bt_io_get(chan, &err, 
                BT_IO_OPT_IMTU, &mtu,
                BT_IO_OPT_CID, &cid, 
                BT_IO_OPT_INVALID);
    if (err) {
        g_error_free(err);
        mtu = ATT_DEFAULT_LE_MTU;
    }

    if (cid == ATT_CID) 
        mtu = ATT_DEFAULT_LE_MTU;

    warnx("Connected: MTU=%d", (int)mtu);    

    g_ble_attrib = g_attrib_new(chan, mtu);
    g_idle_add(ble_listen_start, g_ble_attrib); 

    if (g_ble_disconnected == 2) {
	restore_restart_checkpoint();
	g_op_state = run_state_machine(g_op_state, 0, 0);
    } else {
	if (is_terminal_mode())
	    init_terminal_reader(g_ble_attrib);
    	else
	    g_op_state = run_state_machine(BF_START, 0, 0);
    }
    g_ble_disconnected = 0;
}


static gboolean 
ble_channel_watcher(GIOChannel *source, GIOCondition cond, gpointer user_data)
{
    warnx("BLE Disconnect");
    g_ble_disconnected = 1;
    g_io_channel_unref (source);
    return FALSE;
}


static void
init_ble_reader()
{
    GIOChannel *chan = NULL;
    GError *e = NULL;

    warnx("BLE connecting to: %s", g_ble_mac);
    chan = gatt_connect(
                    "hci0", g_ble_mac,
                    "public", "low", 0, 0,
                    ble_connect_cb, &e);
    if (e != 0) 
	err(1, "BLE connection error");

    g_io_add_watch(chan, G_IO_HUP, ble_channel_watcher, NULL);
}


/****************************************************************/

static gboolean
restart_operations(gpointer user_data)
{
    if (g_checkpoint.valid == 0)
	errx(1, "Device response timeout ...");

    warnx("Device response timeout/error, restarting from last checkpoint");

    if (is_uart_mode()) {
    	restore_restart_checkpoint();
	g_op_state = run_state_machine(g_op_state, 0, 0);
    } else {
	if (g_ble_disconnected == 0) {
    	    restore_restart_checkpoint();
	    g_op_state = run_state_machine(g_op_state, 0, 0);
	} else {
	    g_ble_disconnected = 2;
	    init_ble_reader();
	}
    }

    return G_SOURCE_REMOVE;
}

/****************************************************************/

static gboolean 
uart_read_cb(GIOChannel *source, GIOCondition cond, gpointer user_data)
{
    gchar buf[TX_MTU];
    GError *e = 0;
    gsize rlen;

    disable_response_timer();

    g_io_channel_read_chars(source, buf, TX_MTU, &rlen, &e);

    if (rlen == 0) {
        //g_main_loop_quit(g_event_loop);
        //return true;
        return false; 
    } else if (e != 0) {
	err(1, "UART read error\n");
    }

    if (is_terminal_mode()) {
	write(1, buf, rlen);
    } else {
	g_op_state = run_state_machine(g_op_state, buf, rlen);
    }

    return true;
}


static void
init_uart_reader()
{
   struct termios ti;
   char dummy[16];
   GError *e = 0;

    if ( (g_uart_fd = open(g_uart_fname, O_RDWR)) < 0)
	err(1, "Error opening UART: %s", g_uart_fname);

    if (tcgetattr(g_uart_fd, &ti))
	err(1, "Error reading terminal settings: %s", g_uart_fname);
    
    cfmakeraw(&ti);
    cfsetispeed(&ti, B115200);
    cfsetospeed(&ti, B115200);
    ti.c_lflag = CLOCAL;

    if (tcsetattr(g_uart_fd, TCSAFLUSH, &ti))
	err(1, "Error setting terminal settings: %s", g_uart_fname);

   GIOChannel *wchan = g_io_channel_unix_new(g_uart_fd); /* STDIN */
   g_io_channel_set_encoding(wchan, NULL, &e);
   g_io_channel_set_flags(wchan, G_IO_FLAG_NONBLOCK, &e);
   g_io_add_watch(wchan, G_IO_IN, uart_read_cb, 0);

    if (g_input_fname)
	g_op_state = run_state_machine(BF_START, 0, 0);
    else
	init_terminal_reader(0);
}

/****************************************************************/

static gboolean 
terminal_read_cb(GIOChannel *source, GIOCondition cond, gpointer user_data)
{
    gchar buf[TX_MTU];
    GError *e = 0;
    gsize rlen;

    if (!is_uart_mode() && (g_ble_write_done != 1))
	return true;

    g_io_channel_read_chars(source, buf, TX_MTU, &rlen, &e);

    if (rlen == 0) {
        //g_main_loop_quit(g_event_loop);
        //return true;
        return false; 
    } else if (e != 0) {
	err(1, "Terminal read error");
    }

    if (is_uart_mode()) {
	write(g_uart_fd, buf, rlen);
    } else {
    	GAttrib *attrib = user_data;
    	gatt_request(attrib, g_ble_txhnd, buf, rlen, NULL, NULL);
    }

    return true;
}


static void
init_terminal_reader(void *act)
{
   GError *e = 0;
   GIOChannel *wchan = g_io_channel_unix_new(0); /* STDIN */
   g_io_channel_set_flags(wchan, G_IO_FLAG_NONBLOCK, &e);
   g_io_add_watch(wchan, G_IO_IN, terminal_read_cb, act);
   warnx("Terminal mode active ..");
}


/****************************************************************/

static int
output_file_write(const gchar *buf, int len)
{
    gchar obuf[16];

    memcpy(obuf, buf, len);
    if (g_reverse_input)
    	reverse_bits(obuf, len);
    lseek(g_output_fd, g_offset, SEEK_SET);
    len = write(g_output_fd, obuf, len);
    return len;
}


static int
input_file_load(gchar *buf, int len)
{
    uint64_t r;

    memset(buf, 0, len);
    lseek(g_input_fd, g_offset, SEEK_SET);
    r = read(g_input_fd, buf, len);
    if (r < 0)
	return r;
    if (g_reverse_input)
    	reverse_bits(buf, len);
    return r;
}

/****************************************************************/

static gsize
cmd_set_address(gchar *buf, unsigned int addr)
{
    buf[0] = 'a';
    memcpy(buf + 1, &addr, 4);
    buf[5] = compute_checksum(buf + 1, 4);
    return 6;
}


static gsize
cmd_jmp(gchar *buf)
{
    buf[0] = 'j';
    return 1;
}


static gsize
cmd_activate_ext_mode(gchar *buf)
{
    buf[0] = 1;
    return 1;
}


static gsize
cmd_deactivate_ext_mode(gchar *buf)
{
    buf[0] = 0;
    return 1;
}


static gsize
cmd_write_ext_spi(gchar *buf)
{
    buf[0] = 's';
    memcpy(buf + 1, &g_mem_addr, 4);
    buf[5] = g_ext_checksum;
    return 6;
}


static gsize
cmd_write_ext_buf(gchar *buf)
{
    if ((g_offset & 0xff) == 0) {
	g_ext_checksum = 0;
    }

    buf[0] = 'b';
    buf[1] = g_offset & 0xff;

    if (input_file_load(buf + 2, 16) < 0)
	err(1, "Error reading input file");

    /* No checksum for ext commands as they are only over BLE and
     * BLE has its own checksum. 
     */
    g_ext_checksum ^= compute_checksum(buf + 2, 16);

    return 18;
}


static gsize
cmd_write_file_mem(gchar *buf)
{
    memset(buf, 0, 18);
    buf[0] = 'w';
    if (input_file_load(buf + 1, 16) < 0)
	err(1, "Error reading input file");
    buf[17] = compute_checksum(buf + 1, 16);
    return 18;
}


static gsize
cmd_read_mem(gchar *buf)
{
    buf[0] = 'r';
    return 1;
}


static gsize
cmd_flash_copy(gchar *buf, unsigned int flash_addr)
{
    int len = FLASH_COPY_MAXLEN; 
    buf[0] = 'W';
    memcpy(buf + 1, &flash_addr, 4);
    memcpy(buf + 5, &len, 4);
    buf[9] = compute_checksum(buf + 1, 8);
    return 10;
}


/****************************************************************/

static int
validate_resp(const char *input, int len, char resp)
{
    if (!input || (len != 1) || (input[0] != resp))
	return 1;
    return 0;
}


static int
validate_read_resp(const char *input, int len)
{
    if (!input || (len != 17) || compute_checksum(input, 16) != input[16])
	return 1;
    return 0;
}

/****************************************************************/

static void
device_write_handle(gchar *buf, gsize len, uint16_t handle)
{
    if (!is_uart_mode())
    	gatt_write(g_ble_attrib, handle, buf, len, NULL, NULL);
    else
	write(g_uart_fd, buf, len);

    enable_response_timer();
}

static void
device_write(gchar *buf, gsize len)
{
    device_write_handle(buf, len, g_ble_txhnd);
}

/****************************************************************/

static void
update_done_pct(int mode)
{
    int l, x;
    const char *m;

    switch(mode) {
    case UPDATE_DPCT_UPLOAD:
	l = g_input_len;
	m = "Upload";
	break;
    case UPDATE_DPCT_DOWNLOAD:
	l = g_output_len;
	m = "Download";
	break;
    case UPDATE_DPCT_COPY:
	l = g_flash_copy_len;
	m = "Copy";
	break;
    case UPDATE_DPCT_EXTUPLOAD:
	l = g_input_len;
	m = "Ext. Upload";
	break;
    default:
	errx(1, "Logic error: update_done_pct: unknown mode"); 
    }

    x = (g_offset * 100) / l;
    if (x != g_done_pct) {
	warnx("%s: %d%%", m, x);
	g_done_pct = x;
    }
}


static enum BF_State
run_state_machine(enum BF_State state, const char *input, int input_len)
{
    enum BF_State next_state = state;
    gchar buf[TX_MTU];
    gsize len;
    
    switch(state) {

    case BF_EXT_WRITE_BUF:
	if (g_offset < g_input_len) {
	    /* Checkpoint only at 256 byte boundaries, as BLE firmware
	     * writes in 256 byte blocks
	     */
	    if ((g_offset & 0xff) == 0)
	    	create_restart_checkpoint(BF_EXT_WRITE_BUF);
	    len = cmd_write_ext_buf(buf);
	    next_state = BF_EXT_WRITE_BUF;
    	    device_write(buf, len);
	} else {
	    warnx("Enabling FPGA via CONF and CE pins");
	    len = cmd_deactivate_ext_mode(buf);
	    device_write_handle(buf, len, g_ble_exthnd);
	    next_state = BF_EXT_DONE;
	}
	break;

    case BF_EXT_WRITE_SPI:
	len = cmd_write_ext_spi(buf);
	next_state = BF_EXT_WRITE_SPI_WAIT_OK;
	device_write(buf, len);
	break;

    case BF_EXT_WRITE_SPI_WAIT_OK:
	if (validate_resp(input, input_len, RESP_OK)) {
	    if (g_checkpoint.valid) {
		restart_operations(0);
		break;
	    } else {
	   	errx(1, "BLE ext write: response error: %d %c", 
		    input_len, 
		    (input ? input[0] : 'X'));
	    }
	}

	g_mem_addr += 256;
	next_state = run_state_machine(BF_EXT_WRITE_BUF, 0, 0);
	break;

    case BF_START:
	delete_restart_checkpoint();
	warnx("Begining Operation");
	if (!is_uart_mode() && is_ext_write_mode()) {
	    len = cmd_activate_ext_mode(buf);
	    device_write_handle(buf, len, g_ble_exthnd);
	} else {
	    next_state = run_state_machine(BF_SETADDR, 0, 0);
	}
	break;

    case BF_SETADDR:
	len = cmd_set_address(buf, g_mem_addr + g_offset);
	next_state = BF_SETADDR_WAIT_OK;
	device_write(buf, len);
	break;

    case BF_SETADDR_WAIT_OK:
	if (validate_resp(input, input_len, RESP_OK)) {
	    if (g_checkpoint.valid) {
		restart_operations(0);
		break;
	    } else {
	   	errx(1, "BLE write: set address response error: %d %c", 
		    input_len, 
		    (input ? input[0] : 'X'));
	    }
	}

	if (is_read_mode()) {
	    next_state = run_state_machine(BF_READ_DATA, 0, 0);
	} else {
	    next_state = run_state_machine(BF_WRITE_DATA, 0, 0);
	}

	break;

    case BF_WRITE_WAIT_OK:
	if (validate_resp(input, input_len, RESP_OK)) {
	    if (g_checkpoint.valid) {
		restart_operations(0);
		break;
	    } else {
	   	errx(1, "BLE write: response error: %d %c", 
		    input_len, 
		    (input ? input[0] : 'X'));
	    }
	}

	g_offset += 16;
	update_done_pct(UPDATE_DPCT_UPLOAD);
	next_state = run_state_machine(BF_WRITE_DATA, 0, 0);
	break;

    case BF_READ_WAIT_OK:
	if (validate_read_resp(input, input_len)) {
	    if (g_checkpoint.valid) {
		restart_operations(0);
		break;
	    } else {
		errx(1, "BLE read: response error: %d %c", 
			input_len, 
		    	(input ? input[0] : 'X'));
	    }
	}		

	output_file_write(input, 16);
	g_offset += 16;
	update_done_pct(UPDATE_DPCT_DOWNLOAD);
	next_state = run_state_machine(BF_READ_DATA, 0, 0);
	break;

    case BF_READ_DATA:
	if (g_offset < g_output_len) {
	    create_restart_checkpoint(BF_SETADDR);
	    len = cmd_read_mem(buf);
    	    device_write(buf, len);
	    next_state = BF_READ_WAIT_OK;
	} else {
	    errx(0, "BLE file read finished");
	}
	break;

    case BF_WRITE_DATA:
	if (g_offset < g_input_len) {
	    /* Checkpoint only at 256 byte boundaries, as BLE firmware
	     * writes in 256 byte blocks
	     */
	    if ((g_offset & 0xff) == 0)
	    	create_restart_checkpoint(BF_SETADDR);
	    len = cmd_write_file_mem(buf);
    	    device_write(buf, len);
	    next_state = BF_WRITE_WAIT_OK;
	} else {
	    delete_restart_checkpoint();
	    if (g_jmp_set) {
		warnx("Activating jmp ...");
	   	len = cmd_set_address(buf, g_jmp_addr); 
    		device_write(buf, len);
		next_state = BF_JMP_WAIT_OK;
	    } else if (g_flash_copy_mode && !is_read_mode()) {
		warnx("Begining Flash copy ...");
		g_done_pct = 0;
	    	g_flash_copy_len = g_input_len;
	    	g_offset = 0;
		g_flash_copy_addr_set = 0;
		next_state = run_state_machine(BF_FLASH_COPY, 0, 0);
	    } else
		errx(0, "BLE file write finished");
	}
	break;

    case BF_JMP_WAIT_OK:
	if (validate_resp(input, input_len, RESP_OK))
	    errx(1, "BLE jmp: response error: %d %c", 
		    input_len, 
		    (input ? input[0] : 'X'));

	len = cmd_jmp(buf); 
    	device_write(buf, len);
	next_state = BF_DONE_WAIT_OK;
	break;

    case BF_DONE_WAIT_OK:
	if (input_len <= 0 || input[0] != 'O')
	    errx(1, "BLE jmp: response error: %d %c", 
		    input_len, 
		    (input ? input[0] : 'X'));

	warnx("DONE!"); 
	if (!g_jmp_set)
	    exit(0);
	set_terminal_mode();
	init_terminal_reader(g_ble_attrib);
	break;

    case BF_FLASH_COPY_WAIT_OK:
	if (validate_resp(input, input_len, RESP_OK)) {
	    if (g_checkpoint.valid) {
		restart_operations(0);
		break;
	    } else {
	    	errx(1, "BLE flash copy: response error: %d %c", 
		    input_len, 
		    (input ? input[0] : 'X'));
	    }
	}

	if (g_flash_copy_addr_set == 0) {
	    g_flash_copy_addr_set = 1;
	} else {
	    g_flash_dst_addr += FLASH_COPY_MAXLEN;
	    g_flash_memsrc_addr += FLASH_COPY_MAXLEN;
	    g_offset += FLASH_COPY_MAXLEN;
	    g_flash_copy_addr_set = 0;
	}

	update_done_pct(UPDATE_DPCT_COPY);
	next_state = run_state_machine(BF_FLASH_COPY, 0, 0);
	break;

    case BF_FLASH_COPY:
	if (g_offset < g_flash_copy_len) {
	    if (g_flash_copy_addr_set == 0) {
		create_restart_checkpoint(BF_FLASH_COPY);
	    	len = cmd_set_address(buf, g_flash_memsrc_addr); 
	    } else {
	    	len = cmd_flash_copy(buf, g_flash_dst_addr);
	    }
	    device_write(buf, len);
	    next_state = BF_FLASH_COPY_WAIT_OK;
	} else {
	    errx(0, "BLE flash copy done");
	}
	break;
    }

    return next_state;
}



int
main(int argc, char **argv)
{
    parse_args(argc, argv);

    g_offset = 0;

    if (g_input_fname) {
	if ( (g_input_fd = open(g_input_fname, O_RDONLY)) < 0)
	    err(1, "Error opening input file: %s", g_input_fname);
	g_input_len = lseek(g_input_fd, 0, SEEK_END);
	/* round g_input_len to 256 byte boundary */
	g_input_len = ((g_input_len + 255) / 256) * 256;
    }

    if (is_read_mode()) {
	if ( (g_output_fd = open(g_output_fname, 
				O_WRONLY | O_CREAT | O_TRUNC, 
				S_IRUSR | S_IWUSR)) < 0)
	{
	    err(1, "Error opening output file: %s", g_output_fname);
	}
	g_offset = 0;
    }

    if (is_uart_mode()) {
	init_uart_reader();
    } else {
	init_ble_reader();
    }

    g_event_loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(g_event_loop);
    g_main_loop_unref(g_event_loop);
    return 0;
}
