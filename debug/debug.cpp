#include <iostream>
#include "libwysm.cpp" // Assurez-vous que le chemin est correct

int main() {
    /*try {
        VMWriter writer("loop_program.bin");

        // Initialiser le compteur à 0 (MOV r0, 0)
        writer.add_instruction(0x04, 8, 0, 1, 0x00, 0x00); // MOV r0, 0

        // Début de la boucle
        uint16_t loop_start = 1 * 8; // Adresse de l'instruction de début de la boucle

        // Comparer le compteur à 1000 (CMP r0, 1000)
        writer.add_instruction(0x0A, 8, 0, 1, 0x00, 150); // CMP r0, 1000

        // Sauter à la fin de la boucle si le compteur est égal à 1000 (JE end)
        uint16_t end_address = 5 * 8; // Adresse de l'instruction de fin de la boucle
        writer.add_instruction(0x0B, 8, 0, 1, 0x00, end_address); // JE end

        // Incrémenter le compteur (ADD r0, 1)
        writer.add_instruction(0x05, 8, 0, 1, 0x00, 0x01); // ADD r0, 1

        // Sauter au début de la boucle (JMP loop_start)
        writer.add_instruction(0x09, 8, 0, 1, 0x00, loop_start); // JMP loop_start

        // Fin de la boucle (HALT)
        writer.add_instruction(0xFF, 8, 0, 1, 0x00, 0x00); // HALT

        // Écrire les instructions dans le fichier
        writer.write_to_file();

        std::cout << "Executable created successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }*/

    VMWriter writer("test.bin");
    for (int i = 0; i < 100000; i++) {
      writer.add_instruction(0, 0, 0, 0, 0, 0);
    }
    writer.write_to_file();

    return 0;
}