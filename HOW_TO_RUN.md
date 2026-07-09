# Sudoku Generator & Solver - Quick Start Guide

## How to run the project

Follow these step-by-step instructions to run the project on your machine:

1. **Open your Terminal:**
   - On Windows, press `Win + R`, type `cmd` or `powershell`, and press Enter.

2. **Navigate to the Project Folder:**
   - In the terminal, type the following command to go to the folder where the project is located:
     ```bash
     cd /d d:\DAAel
     ```

3. **Compile the code (Optional):**  
   *(Note: `sudoku.exe` is already included in the folder, so you only need to run this command if you modify the code)*
   - Run this command in the terminal to compile:
     ```bash
     gcc -O3 -o sudoku.exe main.c sudoku.c solver.c server.c gui_html.c -lws2_32
     ```

4. **Start the application:**
   - Run the executable by typing this in the terminal and pressing Enter:
     ```bash
     .\sudoku.exe
     ```

5. **Access the Web Interface:**
   - Once the application is running in the terminal, open your web browser (like Chrome, Edge, or Firefox) and navigate to:
     http://localhost:8080
