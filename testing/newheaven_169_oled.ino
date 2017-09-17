/*
 * testing code for Newhaven 160x128 Color OLED (SEPS525).
 *
 */

#define   SDI_PIN   13    // SDI (serial mode) signal connected to D7
#define   SCL_PIN   14    // SCL (serial mdoe) signal connected to D5
#define    RS_PIN    5    // RS (D/C) signal connected D1
#define   RES_PIN    0    // /RES signal connected to D3
#define    CS_PIN   15    // /CS signal connected to D8

#define COMMAND_WriteMemoryStart 0x22

#include "fonttables.h"

void clocked_sdi(unsigned char level) {
	digitalWrite(SCL_PIN, LOW);
	digitalWrite(SDI_PIN, level?HIGH:LOW);
	digitalWrite(SCL_PIN, HIGH);
}

// send 8 command bits
void send_command(unsigned char cmd) {
	digitalWrite(CS_PIN, LOW);
	digitalWrite(RS_PIN, LOW);
	clocked_sdi(0x80 & cmd);
	clocked_sdi(0x40 & cmd);
	clocked_sdi(0x20 & cmd);
	clocked_sdi(0x10 & cmd);
	clocked_sdi(0x08 & cmd);
	clocked_sdi(0x04 & cmd);
	clocked_sdi(0x02 & cmd);
	clocked_sdi(0x01 & cmd);
	digitalWrite(CS_PIN, HIGH);
}

// send 8 data bits
void send_8bit_data(unsigned char data) {
	digitalWrite(CS_PIN, LOW);
	digitalWrite(RS_PIN, HIGH);
	clocked_sdi(0x80 & data);
	clocked_sdi(0x40 & data);
	clocked_sdi(0x20 & data);
	clocked_sdi(0x10 & data);
	clocked_sdi(0x08 & data);
	clocked_sdi(0x04 & data);
	clocked_sdi(0x02 & data);
	clocked_sdi(0x01 & data);
	digitalWrite(CS_PIN, HIGH);
}

// send 8bit command followed by 8bit data
void send_command_with_data(unsigned char cmd, unsigned char data) {
	send_command(cmd);
	send_8bit_data(data);
}

// write 6 data bit (colored pixel)
void send_6bit_data(unsigned char data) {
	digitalWrite(CS_PIN, LOW);
	digitalWrite(RS_PIN, HIGH);
	clocked_sdi(0x80 & data);
	clocked_sdi(0x40 & data);
	clocked_sdi(0x20 & data);
	clocked_sdi(0x10 & data);
	clocked_sdi(0x08 & data);
	clocked_sdi(0x04 & data);
	digitalWrite(CS_PIN, HIGH);
}

// set row range
void set_row_range(unsigned char y_start, unsigned char y_end) {
	send_command(0x19);
	send_8bit_data(y_start);
	send_command(0x1A);
	send_8bit_data(y_end);
}

// set column range
void set_column_range(unsigned char x_start, unsigned char x_end) {
	send_command(0x17);
	send_8bit_data(x_start);
	send_command(0x18);
	send_8bit_data(x_end);
}

// draw a single character/letter to a given position
// in a given color with a given backgroundcolor
void draw_letter(unsigned char x_pos, unsigned char y_pos,
		unsigned char letter, unsigned long textColor,
		unsigned long backgroundColor) {
	int y;
	int x;

	for(y=0;y<8;y++) {    // 8 pixel height
		set_position(x_pos,y_pos+y);
		send_command(COMMAND_WriteMemoryStart);
		for (x=0;x<5;++x) {  // 5 pixel wide
			draw_pixel((Ascii_1[letter][x] & (0x80>>y))
					?textColor:backgroundColor);
		}
	}
}

// write one pixel of a given color
void draw_pixel(unsigned long color) {
	send_6bit_data(color>>16);
	send_6bit_data(color>>8);
	send_6bit_data(color);
}

// set x,y address
void set_position(unsigned char x_pos, unsigned char y_pos) {
	send_command(0x20);
	send_8bit_data(x_pos);
	send_command(0x21);
	send_8bit_data(y_pos);
}

// fill screen with a given color
void fillscreen(unsigned long color) {
	unsigned int i;
	set_position(0,0);
	send_command(COMMAND_WriteMemoryStart);
	for(i=0;i<160*128;i++) {
		draw_pixel(color);
	}
}

void setup() {
	pinMode(CS_PIN, OUTPUT);
	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(RES_PIN, OUTPUT);
	pinMode(RS_PIN, OUTPUT);
	pinMode(SCL_PIN, OUTPUT);
	pinMode(SDI_PIN, OUTPUT);

	digitalWrite(CS_PIN, HIGH);
	digitalWrite(SDI_PIN, LOW);
	digitalWrite(SCL_PIN, LOW);
}

void oled_init() {
	digitalWrite(RES_PIN, LOW);
	delay(500);
	digitalWrite(RES_PIN, HIGH);
	delay(500);

	send_command_with_data(0x04, 0x03);
	delay(2);

	send_command_with_data(0x04, 0x00);
	delay(2);

	send_command_with_data(0x3B, 0x00);
	send_command_with_data(0x02, 0x01);
	send_command_with_data(0x03, 0x90);
	send_command_with_data(0x80, 0x01);
	send_command_with_data(0x08, 0x04);
	send_command_with_data(0x09, 0x05);
	send_command_with_data(0x0A, 0x05);
	send_command_with_data(0x0B, 0x9D);
	send_command_with_data(0x0C, 0x8C);
	send_command_with_data(0x0D, 0x57);
	send_command_with_data(0x10, 0x56);
	send_command_with_data(0x11, 0x4D);
	send_command_with_data(0x12, 0x46);
	send_command_with_data(0x13, 0xA0);
	send_command_with_data(0x14, 0x01);
	send_command_with_data(0x16, 0x76);
	send_command_with_data(0x28, 0x7F);
	send_command_with_data(0x29, 0x00);
	send_command_with_data(0x06, 0x01);
	send_command_with_data(0x05, 0x00);
	send_command_with_data(0x15, 0x00);

	set_column_range(0, 159);
	set_row_range(0, 127);
}

void loop() {
	oled_init();

	fillscreen(0x000000); // clear screen

	// write some hello world
	draw_letter(10, 50, 40, 0xFFFFFF, 0x000000);
	draw_letter(17, 50, 69, 0xFFFFFF, 0x000000);
	draw_letter(24, 50, 76, 0xFFFFFF, 0x000000);
	draw_letter(31, 50, 76, 0xFFFFFF, 0x000000);
	draw_letter(38, 50, 79, 0xFFFFFF, 0x000000);
	draw_letter(45, 50, 0, 0xFFFFFF, 0x000000);
	draw_letter(52, 50, 55, 0xFFFFFF, 0x000000);
	draw_letter(59, 50, 79, 0xFFFFFF, 0x000000);
	draw_letter(66, 50, 82, 0xFFFFFF, 0x000000);
	draw_letter(73, 50, 76, 0xFFFFFF, 0x000000);
	draw_letter(80, 50, 68, 0xFFFFFF, 0x000000);
	draw_letter(87, 50, 1, 0xFFFFFF, 0x000000);

	for (;;) {
		delay(1000);
	}
}
