# ASCII Video Player
A tool written in C which lets you play any video (local or with URL) as ASCII text in the terminal. Created as a project for the CSE-201 Procedural Programming in C course.

## Demo
https://github.com/user-attachments/assets/e9a8dd83-5fdd-43d1-8c6f-0fb773e63b5f


## Usage
- Install the dependencies

  - Ubuntu/Debian:
    ```
    sudo apt-get install libavcodec-dev libavformat-dev libswscale-dev
    ```
  - Arch:
    ```
    sudo pacman -S ffmpeg
    ```
- Compile the program ```gcc main.c -o player -lavformat -lavcodec -lavutil -lswscale -lm```

- Then execute the `player` file with `./player`. The program will ask for the name of the file, you can either enter the name of a video file in the same directory or feed it a URL to a video.

**Note**: The video will resize itself to the current dimensions of your terminal, if you wish to have a higher quality output zoom out your terminal before running the program
