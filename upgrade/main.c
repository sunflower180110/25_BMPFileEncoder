#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "bmp.h"

#define MAX_FILE_NAME_LENGTH 500   
#define MAX_MESSAGE_LENGTH 1000

void print_help_message(void)
{
    printf("--- Available Commands ---\n");
    printf("  -h <input_bmp>                             : Display BMP header information\n");
    printf("  -o <input_bmp>                             : Output BMP file data in hexadecimal format\n");
    printf("  -g <input_bmp> <output_bmp>                : Convert BMP image to grayscale\n");
    printf("  -e <input_bmp> <message_file> <output_bmp> : Encode message into BMP image\n");
    printf("  -d <input_bmp>                             : Decode hidden message from BMP image\n");
    printf("  -help                                      : Display this help message\n");
}

// free BMP image data
void free_bmp_image(BMPImage *img)
{
    if (img != NULL)
    {
        if (img->data != NULL)
        {
            free(img->data);
        }
    }
}

// print BMP file header information
void print_header_info(const BMPHeader *header)
{
    printf("\n--- BMP file header information ---\n");
    printf("type: 0x%04x\n", header->type);
    printf("size: %u bytes\n", header->size);
    printf("reserved1: %u\n", header->reserved1);
    printf("reserved2: %u\n", header->reserved2);
    printf("offset: %u bytes\n", header->offset);
    printf("DIB header size: %u bytes\n", header->dib_header_size);
    printf("width: %d px\n", header->width_px);
    printf("height: %d px\n", header->height_px);
    printf("number of planes: %u\n", header->num_planes);
    printf("bits per pixel: %u\n", header->bits_per_pixel);
    printf("compression: %u\n", header->compression);
    printf("image size: %u bytes\n", header->image_size_bytes);
    printf("X resolution: %d ppm\n", header->x_resolution_ppm);
    printf("Y resolution: %d ppm\n", header->y_resolution_ppm);
    printf("number of colors: %u\n", header->num_colors);
    printf("important colors: %u\n", header->important_colors);
}

// print BMP file data information
void print_data_hex(const BMPImage *img)
{
    // size of header (54bytes)
    int header_size = sizeof(BMPHeader);
    // size of image data
    int data_size = (int)img->header.size - img->header.offset;
    // size of total BMP (header + image)
    int total_size = header_size + data_size;

    printf("\n--- BMP file data (hex dump) ---\n");

    unsigned char *header_bytes = (unsigned char *)&img->header;
    unsigned char *data = img->data;
    unsigned char byte;
    int offset = 0; // current offset in bytes

    // print header (54bytes)
    for (int i = 0; i < header_size; i++)
    {
        if (offset % 16 == 0)
        {
            if (offset > 0)
            {
                // print ASCII expression of before sentence
                printf("  |");
                int ascii_start = offset - 16;
                for (int j = ascii_start; j < offset; j++)
                {
                    byte = (j < header_size) ? header_bytes[j] : data[j - header_size];
                    // if byte is unprintable, then print '.'
                    printf("%c", (byte >= 32 && byte <= 126) ? byte : '.');
                }
                printf("|\n");
            }
            printf("%08x: ", offset);
        }

        // print byte in hexadecimal
        printf("%02x", header_bytes[i]);

        // add space every 2 bytes (4 hex digits)
        if (offset % 2 == 1)
        {
            printf(" ");
        }

        offset++;
    }

    // print data section (image data)
    for (int i = 0; i < data_size; i++)
    {
        if (offset % 16 == 0)
        {
            if (offset > 0)
            {
                // print ASCII representation of previous line
                printf("  |");
                int ascii_start = offset - 16;
                for (int j = ascii_start; j < offset; j++)
                {
                    if (j < header_size)
                    {
                        byte = header_bytes[j];
                    }
                    else
                    {
                        byte = data[j - header_size];
                    }
                    // if byte is unprintable, then print '.'
                    printf("%c", (byte >= 32 && byte <= 126) ? byte : '.');
                }
                printf("|\n");
            }
            printf("%08x: ", offset);
        }

        // print byte in hexadecimal
        printf("%02x", data[i]);

        // add space every 2 bytes (4 hex digits)
        if (offset % 2 == 1)
        {
            printf(" ");
        }

        offset++;
    }

    // if the last line is not complete (less than 16 bytes), fill the rest with spaces
    int remaining = total_size % 16;
    if (remaining != 0)
    {
        // fill with spaces for hexadecimal output
        for (int k = 0; k < 16 - remaining; k++)
        {
            printf("  "); // 2글자 공간
            if ((remaining + k) % 2 == 1)
                printf(" "); // 2바이트마다 공백
        }
    }

    // print ASCII representation of last line
    printf("  |");
    int start_ascii = total_size - remaining;
    if (remaining == 0 && total_size > 0)
        start_ascii = total_size - 16;
    for (int j = start_ascii; j < total_size; j++)
    {

        if (j < header_size)
        {
            byte = header_bytes[j];
        }
        else
        {
            byte = data[j - header_size];
        }
        printf("%c", (byte >= 32 && byte <= 126) ? byte : '.');
    }
    printf("|\n");
}

// calculate padding size for each row
int calculate_padding(int width_px)
{
    // 3bytes per pixel (24-bit BMP)
    int row_size_bytes = width_px * 3;

    return (4 - (row_size_bytes % 4)) % 4;
}

// convert 24-bit BMP image to grayscale
int convert_to_grayscale(BMPImage *img)
{
    if (img->header.bits_per_pixel != 24)
    {
        fprintf(stderr, "Error: This operation only supports uncompressed 24-bit BMP\n");
        return 2; // Invalid Arguments
    }

    int width = img->header.width_px;        // width in pixels
    int height = abs(img->header.height_px); // height in pixels
    int padding = calculate_padding(width);  // row padding in bytes
    unsigned char *data = img->data;
    int row_start_offset = 0; // start offset of the current row in data array

    printf("\n--- Converting to Grayscale ---\n");
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            // each pixel's BGR data offset
            int pixel_offset = row_start_offset + (j * 3);
            unsigned char blue = data[pixel_offset + 0];
            unsigned char green = data[pixel_offset + 1];
            unsigned char red = data[pixel_offset + 2];

            // calculate grayscale value using green channel
            data[pixel_offset + 0] = green; // Blue = Green
            data[pixel_offset + 2] = green; // Red = Green
        }

        // skip padding bytes and move offset to the start of the next row
        row_start_offset += (width * 3) + padding;
    }

    printf("successfully converted to grayscale\n");
    return 0; // return 0 for success
}

// hide message into BMP image using LSB steganography
int encode_message(BMPImage *img, char message_file[])
{
    if (img->header.bits_per_pixel != 24 || img->header.compression != 0)
    {
        fprintf(stderr, "Error: This operation only supports uncompressed 24-bit BMP\n");
        return 2; // Invalid Arguments
    }

    // read message from file
    FILE *message_file_ptr = fopen(message_file, "r");
    if (message_file_ptr == NULL)
    {
        fprintf(stderr, "Error: filename '%s' is not incorrect\n", message_file);
        return 1; // File Not Found
    }
    char message[MAX_MESSAGE_LENGTH + 1];
    if (fgets(message, sizeof(message), message_file_ptr) == NULL)
    {
        fprintf(stderr, "Error: reading message from file failed\n");
        fclose(message_file_ptr);
        return 1; // File Not Found
    }

    // remove newline character if present
    int len = strlen(message);
    if (len > 0 && message[len - 1] == '\n')
    {
        message[len - 1] = '\0';
    }

    // total size of image data in bytes
    int data_size = (int)img->header.size - img->header.offset;
    int msg_len = strlen(message);

    // total bits required to encode message (1 byte for length + 8 bits per character)
    int required_bits = 8 + (msg_len * 8);
    int max_bits = data_size * 1; // 1 bit per byte of image data

    printf("\n--- encode message ---\n");
    if (required_bits > max_bits)
    {
        fprintf(stderr, "Error: Message is too long\n");
        fclose(message_file_ptr);
        return 2; // Invalid Arguments
    }

    unsigned char *data = img->data;
    int data_index = 0; // current index in BMP data array

    // encode message length (1 byte)
    int len_byte = msg_len;
    for (int i = 0; i < 8; i++)
    { // use 8 bytes of image data for 8 bits of length
        if (data_index >= data_size)
        {
            fprintf(stderr, "Error: Insufficient space while encoding length\n");
            fclose(message_file_ptr);
            return 2; // Invalid Arguments
        }

        // extract i-th bit of len_byte
        int msg_bit = (len_byte >> i) & 1;

        // delete LSB of data byte and set it to msg_bit
        data[data_index] &= 0xFE;    // delete LSB
        data[data_index] |= msg_bit; // set LSB

        data_index++;
    }

    // encode message characters
    for (int i = 0; i < msg_len; i++)
    {
        unsigned char current_char = message[i];

        for (int j = 0; j < 8; j++)
        {
            if (data_index >= data_size)
            {
                fprintf(stderr, "Error: Insufficient space while encoding message\n");
                fclose(message_file_ptr);
                return 2; // Invalid Arguments
            }

            // extract j-th bit of current_char
            int msg_bit = (current_char >> j) & 1;

            // delete LSB of data byte and set it to msg_bit
            data[data_index] &= 0xFE;
            data[data_index] |= msg_bit;

            data_index++;
        }
    }
    printf("message file \'%s\' is successfully encoded into image\n", message_file);
    fclose(message_file_ptr);
    return 0; // return 0 for success
}

// decode hidden message from BMP image using LSB steganography
int decode_message(const BMPImage *img)
{
    if (img->header.bits_per_pixel != 24 || img->header.compression != 0)
    {
        fprintf(stderr, "Error: This operation only supports uncompressed 24-bit BMP\n");
        return 2; // Invalid Arguments
    }
    // total size of image data in bytes
    int data_size = (int)img->header.size - img->header.offset;
    unsigned char *data = img->data;
    int data_index = 0; // current index in BMP data array

    char message[MAX_MESSAGE_LENGTH + 1]; // buffer for decoded message

    printf("\n--- decode message ---\n");
    // Decode message length (1 byte) ---
    int msg_len = 0;
    for (int i = 0; i < 8; i++)
    { // use 8 bytes of image data for 8 bits of length
        if (data_index >= data_size)
        {
            fprintf(stderr, "Error: Insufficient space while decoding length\n");
            return 2; // Invalid Arguments
        }

        // extract LSB from data byte
        int bit = data[data_index] & 1;

        // set the i-th bit of msg_len
        msg_len |= (bit << i);

        data_index++;
    }

    printf("Decoded message length: %d bytes\n", msg_len);

    // Read the message from the image data
    for (int i = 0; i < msg_len; i++)
    {
        if (data_index >= data_size)
        {
            fprintf(stderr, "Error: Insufficient space while decoding message\n");
            return 2; // Invalid Arguments
        }

        unsigned char current_char = 0;
        for (int j = 0; j < 8; j++)
        {
            // Extract LSB from data byte
            int bit = data[data_index] & 1;
            current_char |= (bit << j);
            data_index++;
        }
        message[i] = current_char;
    }

    message[msg_len] = '\0'; // Null-terminate the string
    printf("Hidden message: \"%s\"\n", message);
    return 0; // return 0 for success
}

// read BMP file from disk into BMPImage structure
int read_bmp(const char *filename, BMPImage *img)
{
    FILE *file = fopen(filename, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "Error: filename \'%s\' is incorrect\n", filename);
        return 1; // File Not Found
    }

    // read 54-byte BMP header
    if (fread(&img->header, sizeof(BMPHeader), 1, file) != 1)
    {
        fprintf(stderr, "Error: reading BMP header\n");
        fclose(file);
        return 1; // File Not Found
    }

    // check BMP magic number (0x4D42)
    if (img->header.type != 0x4D42)
    {
        fprintf(stderr, "Error: File is not a valid BMP format (magic number 0x%X)\n", img->header.type);
        fclose(file);
        return 2; // Invalid Arguments
    }

    // Check for 24-bit uncompressed BMP
    // Only support 24-bit BMP with no compression
    if (img->header.bits_per_pixel != 24 || img->header.compression != 0)
    {
        fprintf(stderr, "Error: This operation only supports uncompressed 24-bit BMP\n");
        fclose(file);
        return 2; // Invalid Arguments
    }

    // calculate the size of the pixel data array (header.size - header.offset)
    int data_size = (int)img->header.size - img->header.offset;

    // allocate memory for the pixel data
    img->data = (unsigned char *)malloc(data_size);
    if (!img->data)
    {
        fprintf(stderr, "Error: image data memory allocation failed\n");
        fclose(file);
        return 3; // Memory Allocation Failure
    }

    // move to the start of the pixel data array
    if (fseek(file, img->header.offset, SEEK_SET) != 0)
    {
        fprintf(stderr, "Error: seeking to image data offset failed\n");
        free(img->data);
        fclose(file);
        return 1; // File Not Found
    }

    // read the pixel data into the allocated memory
    if (fread(img->data, 1, data_size, file) != data_size)
    {
        fprintf(stderr, "Error: reading image data failed\n");
        free(img->data);
        fclose(file);
        return 1; // File Not Found
    }

    fclose(file);
    return 0; // return 0 for success
}

// write BMPImage structure to BMP file on disk
int write_bmp(const char *filename, const BMPImage *img)
{
    FILE *file = fopen(filename, "wb");
    if (file == NULL)
    {
        fprintf(stderr, "Error: filename \'%s\' is incorrect\n", filename);
        return 1; // File Not Found
    }

    // write BMP header
    if (fwrite(&img->header, sizeof(BMPHeader), 1, file) != 1)
    {
        fprintf(stderr, "Error: writing BMP header failed\n");
        fclose(file);
        return 1; // File Not Found
    }

    // move to the start of the pixel data array
    if (fseek(file, img->header.offset, SEEK_SET) != 0)
    {
        fprintf(stderr, "Error: seeking to image data offset failed\n");
        fclose(file);
        return 1; // File Not Found
    }

    // calculate the size of the pixel data array (header.size - header.offset)
    int data_size = (int)img->header.size - img->header.offset;

    // write the pixel data from the BMPImage structure to the file
    if (fwrite(img->data, 1, data_size, file) != data_size)
    {
        fprintf(stderr, "Error: writing image data failed\n");
        fclose(file);
        return 1; // File Not Found
    }

    fclose(file);
    return 0; // return 0 for success
}

// parse command line arguments and return option character
char parse_command_line(int argc, char *argv[], char *input_bmp, char *grayscale_output, char *stego_output, char *message_file)
{
    if (argc < 2)
    {
        fprintf(stderr, "Error: option parsing failed\n");
        print_help_message();
        return '\0'; // return null character to indicate error
    }

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-g") == 0 && i + 2 < argc)
        {
            strcpy(input_bmp, argv[i + 1]);
            strcpy(grayscale_output, argv[i + 2]);
            return 'g';
        }
        else if (strcmp(argv[i], "-e") == 0 && i + 3 < argc)
        {
            strcpy(input_bmp, argv[i + 1]);
            strcpy(message_file, argv[i + 2]);
            strcpy(stego_output, argv[i + 3]);
            return 'e';
        }
        else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc)
        {
            strcpy(input_bmp, argv[i + 1]);
            return 'd';
        }
        else if (strcmp(argv[i], "-h") == 0 && i + 1 < argc)
        {
            strcpy(input_bmp, argv[i + 1]);
            return 'h';
        }
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc)
        {
            strcpy(input_bmp, argv[i + 1]);
            return 'o';
        }

        else if (strcmp(argv[i], "-help") == 0)
        {
            return 'H';
        }
    }

    fprintf(stderr, "Error: option parsing failed\n");
    print_help_message();
    return '\0'; // return null character to indicate error
}

int main(int argc, char *argv[])
{
    char input_bmp[MAX_FILE_NAME_LENGTH];
    char grayscale_output[MAX_FILE_NAME_LENGTH];
    char stego_output[MAX_FILE_NAME_LENGTH];
    char message_file[MAX_FILE_NAME_LENGTH];
    char option;

    BMPImage bmp_img;

    // command line parsing
    option = parse_command_line(argc, argv, input_bmp, grayscale_output, stego_output, message_file);

    // quit if option parsing failed
    if (option == '\0')
    {
        return 2; // Invalid Arguments
    }

    int read_result;
    switch (option)
    {
    case 'h': // BMP header information
        read_result = read_bmp(input_bmp, &bmp_img);
        if (read_result != 0)
        {
            return read_result; // return read_bmp's error code
        }
        print_header_info(&bmp_img.header);
        free_bmp_image(&bmp_img);
        break;

    case 'o': // BMP data hex dump
        read_result = read_bmp(input_bmp, &bmp_img);
        if (read_result != 0)
        {
            return read_result; // return read_bmp's error code
        }
        print_data_hex(&bmp_img);
        free_bmp_image(&bmp_img);
        break;

    case 'g': // convert to grayscale
        read_result = read_bmp(input_bmp, &bmp_img);
        if (read_result != 0)
        {
            return read_result; // return read_bmp's error code
        }

        int convert_result = convert_to_grayscale(&bmp_img);
        if (convert_result != 0)
        {
            free_bmp_image(&bmp_img);
            return convert_result; // return convert_to_grayscale's error code
        }

        int write_result = write_bmp(grayscale_output, &bmp_img);
        if (write_result != 0)
        {
            free_bmp_image(&bmp_img);
            return write_result; // return write_bmp's error code
        }
        printf("grayscale image is saved to %s\n", grayscale_output);
        free_bmp_image(&bmp_img);
        break;

    case 'e': // hide message using LSB steganography
        read_result = read_bmp(input_bmp, &bmp_img);
        if (read_result != 0)
        {
            return read_result; // return read_bmp's error code
        }

        int encode_result = encode_message(&bmp_img, message_file);
        if (encode_result != 0)
        {
            free_bmp_image(&bmp_img);
            return encode_result; // return encode_message's error code
        }

        write_result = write_bmp(stego_output, &bmp_img);
        if (write_result != 0)
        {
            free_bmp_image(&bmp_img);
            return write_result; // return write_bmp's error code
        }
        free_bmp_image(&bmp_img);
        break;

    case 'd': // decode hidden message
        read_result = read_bmp(input_bmp, &bmp_img);
        if (read_result != 0)
        {
            return read_result; // return read_bmp's error code
        }
        int decode_result = decode_message(&bmp_img);
        if (decode_result != 0)
        {
            free_bmp_image(&bmp_img);
            return decode_result; // return decode_message's error code
        }
        free_bmp_image(&bmp_img);
        break;

    case 'H': // help message
        print_help_message();
        break;

    default:
        fprintf(stderr, "Error: unknown option\n");
        return 2; // Invalid Arguments
    }
    return 0; // return 0 for success
}
