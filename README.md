# ascii_video_player
A tool which lets you play any videos as ascii text in the terminal. Created as a project for the CSE-201 Procedural Programming in C course.


# Frame extractor usage :
- Give the video name with its extension in the program.
- Compile the program using the command.
```
gcc frames_from_video.c -o executable -lavformat -lavcodec -lavutil -lswscale -lm
```

- Then execute the `executable` file. New folder `extracted_frames` will be created with the frames of the video.