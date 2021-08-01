/*
 * captcha.c
 * Generates Single Captcha from a word taken randomly from a list of words and saves it to captcha.png
 */
// Last updated 2008/12/11 17:33

/* There's no equivalent convert command for this. It is a demo of MagickWand.
See this forum thread for the genesis of these effects
http://www.imagemagick.org/discourse-server/viewtopic.php?f=6&t=11586
and Anthony's Text Effects page at: 
http://www.imagemagick.org/Usage/fonts/
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MagickWand/MagickWand.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// Given a pattern name (which MUST have a leading #) and a pattern file,
// set up a pattern URL for later reference in the specified drawing wand
// Currently only used in Text Effect 2 
void captcha_set_tile_pattern(DrawingWand *d_wand, const char *pattern_name, const char *pattern_file)
{
  MagickWand *t_wand;
  long w,h;

  t_wand = NewMagickWand();
  MagickReadImage(t_wand, pattern_file);
  // Read the tile's width and height
  w = MagickGetImageWidth(t_wand);
  h = MagickGetImageHeight(t_wand);

  DrawPushPattern(d_wand, pattern_name+1, 0, 0, w, h);
  DrawComposite(d_wand, SrcOverCompositeOp, 0, 0, 0, 0, t_wand);
  DrawPopPattern(d_wand);
  DrawSetFillPatternURL(d_wand, pattern_name);

  DestroyMagickWand(t_wand);
}

unsigned char *captcha_draw(MagickWand *magick_wand, DrawingWand *d_wand, PixelWand *p_wand, const char *text,
    const char *captcha_file_fmt, int captcha_add_tile_pattern, int captcha_rotate,
    size_t *blob_len)
{
#define pp "pattern:"

  int rpos = 0;
  const char *pattern = NULL;
  double degree = 0;
  

  static const char *patterns[] = {
    pp "CHECKERBOARD",
    pp "GRAY5", pp "GRAY10", pp "GRAY15", pp "GRAY20", pp "GRAY25", pp "GRAY30", pp "GRAY35", pp "GRAY40", pp "GRAY45", pp "GRAY50", pp "GRAY55", pp "GRAY60",
    pp "HORIZONTAL", pp "HORIZONTAL2", pp "HORIZONTAL3",
    //pp "HS_DIAGCROSS",
    pp "LEFT30",
    pp "RIGHT30",
    pp "SMALLFISHSCALES",
    pp "VERTICAL", pp "VERTICAL2", pp "VERTICAL3"
  };
  static int npatterns = sizeof(patterns) / sizeof(const char *);
  //fprintf(stderr, "npatterns=%d\n", npatterns);


  static double degrees[] = {
    -20.0, -18.0, -16.0, -14.0, -12.0,
    -10.0, -8.0, -6.0, -4.0, -2.0,
    2.0, 4.0, 6.0, 8.0, 10.0,
    12.0, 14.0, 16.0, 18.0, 20.0
  };
  static int ndegrees = sizeof(degrees) / sizeof(double);
  //fprintf(stderr, "ndegrees=%d\n", ndegrees);


  // Used for text effect
  double d_args[8];

  /*
#ifdef COLORIZE
  MagickWand *c_wand = NULL;

  PixelSetColor(p_wand,"yellow");
  DrawSetFillColor(d_wand,p_wand);
  cp_wand = NewPixelWand();
  PixelSetColor(cp_wand,"gold");
  MagickColorizeImage(magick_wand,p_wand,cp_wand);
#endif
  // and write it
  MagickWriteImage(magick_wand,"text_bevel.png");

  if(magick_wand)magick_wand = DestroyMagickWand(magick_wand);
  if(d_wand)d_wand = DestroyDrawingWand(d_wand);
  if(p_wand)p_wand = DestroyPixelWand(p_wand);
#ifdef COLORIZE
  if(cp_wand)cp_wand = DestroyPixelWand(cp_wand);
#endif
  */

  ClearPixelWand(p_wand);
  ClearDrawingWand(d_wand);
  ClearMagickWand(magick_wand);

  if (captcha_add_tile_pattern) {
    rpos = random() % npatterns;
    pattern = patterns[rpos];
    captcha_set_tile_pattern(d_wand, "#check", pattern); // https://www.imagemagick.org/script/formats.php#builtin-patterns
  }

  // Create a 320x100 transparent canvas
  PixelSetColor(p_wand,"none");
  MagickNewImage(magick_wand,320,100,p_wand);

  //MagickNewImage(magick_wand,160,70,p_wand);

  // Set up a 72 point font 
  //DrawSetFont (d_wand, "Verdana-Bold-Italic" ) ;
  DrawSetFont (d_wand, "Verdana" ) ;
  //DrawSetFontSize(d_wand,72);
  DrawSetFontSize(d_wand,36);
  // Now draw the text
  //DrawAnnotation(d_wand,25,65,"penguin");
  DrawAnnotation(d_wand, 10, 30, (const unsigned char *) text);
  // Draw the image on to the magick_wand
  MagickDrawImage(magick_wand,d_wand);

  // Trim the image
  MagickTrimImage(magick_wand,0);
  // Add the border
  PixelSetColor(p_wand,"none");
  MagickBorderImage(magick_wand,p_wand,10,10, CopyAlphaCompositeOp);


  //	MagickSetImageMatte(magick_wand,MagickTrue);
  //	MagickSetImageVirtualPixelMethod(magick_wand,TransparentVirtualPixelMethod);
  // 	d_args[0] = 0.1;d_args[1] = -0.25;d_args[2] = -0.25; [3] += .1
  // The first value should be positive. If it is negative the image is *really* distorted
  d_args[0] = 0.0;
  d_args[1] = 0.0;
  d_args[2] = 0.5;
  // d_args[3] should normally be chosen such the sum of all 4 values is 1
  // so that the result is the same size as the original
  // You can override the sum with a different value
  // If the sum is greater than 1 the resulting image will be smaller than the original
  d_args[3] = 1 - (d_args[0] + d_args[1] + d_args[2]);
  // Make the result image smaller so that it isn't as likely
  // to overflow the edges
  // d_args[3] += 0.1;
  // 0.0,0.0,0.5,0.5,0.0,0.0,-0.5,1.9
  //d_args[3] = 0.5;
  d_args[4] = 0.0;
  d_args[5] = 0.0;
  d_args[6] = -0.5;
  d_args[7] = 1.9;
  // DON'T FORGET to set the correct number of arguments here
  MagickDistortImage(magick_wand,BarrelDistortion,8,d_args,MagickTrue);
  //	MagickResetImagePage(magick_wand,"");

  // Trim the image again
  MagickTrimImage(magick_wand,0);
  // Add the border
  PixelSetColor(p_wand,"none");
  MagickBorderImage(magick_wand,p_wand,10,10, CopyAlphaCompositeOp);
  // and write it


  if (captcha_rotate) {
    rpos = random() % ndegrees;;
    degree = degrees[rpos];
    // rotate image to some angle
    MagickRotateImage(magick_wand, p_wand, degree);
  }

  fprintf(stderr, "pattern=%s, degree=%lf, text=%s\n", pattern, degree, text);

  //size_t blob_len = 0;
  //unsigned char *blob = NULL;
  MagickSetImageFormat(magick_wand, captcha_file_fmt);
  MagickResetIterator(magick_wand);
  //blob = MagickGetImageBlob(magick_wand, &blob_len);
  return MagickGetImageBlob(magick_wand, blob_len);
  //fprintf(stderr, "blob_len=%lu\n", blob_len);
  //MagickRelinquishMemory(blob);

#if 0
  MagickWriteImage(magick_wand,"captcha.png");
#endif

  //return blob;
}

int main(void)
{
  const char *word = NULL;
  int rpos = 0;
  const char *cfile = "words_6_8.txt";
  char buf[10];
  char **words = NULL;
  size_t nwords = 0;
  int i = 0;
  size_t tlen = 0;
  FILE *fp = fopen(cfile, "r");
  if (!fp) {
    perror(cfile);
  }
  while (fgets(buf, sizeof(buf), fp)) {
    if (i == 0) {
      i++;
      nwords = strtoul(buf, NULL, 10);
      if (nwords <= 0) {
	fprintf(stderr, "First line of the words file requires to be count for lines in the file\n");
	goto err;
      }
      words = calloc(nwords, sizeof(char *));
      continue;
    }
    tlen = strlen(buf);
    if (buf[tlen-1] == '\n') {
      buf[--tlen] = '\0';
    }
    words[i-1] = strdup(buf);
    //fprintf(stderr, "word[%d]=%s\n", i-1, words[i-1]);
    i++;
  }
  fprintf(stderr, "i=%d, nwords=%lu.\n", i, nwords);
  fclose(fp);

  fprintf(stderr, "Finished loading %s\n", cfile);

  srandom((unsigned int) time(NULL)); // we don't need cryptographically secure random number in Captcha
  MagickWandGenesis();

#define CM 4
#define CN 10
  char choices[CM][CN];
  int tpos = 0;
  int tpos1 = 0;

  fprintf(stderr, "Press ENTER to generate New Captcha. Ctrl-D (EOF) to exit.\n");

  int x = 0;

  int xmax = 1;

  MagickWand *magick_wand = NULL;
  DrawingWand *d_wand = NULL;
  PixelWand *p_wand = NULL;

  magick_wand = NewMagickWand();
  d_wand = NewDrawingWand();
  p_wand = NewPixelWand();


  int fd = 0;
  ssize_t nwritten = 0;
  unsigned char *blob = NULL;
  size_t blob_len = 0;
  const char *captcha_file = "captcha.png";
  const char *captcha_file_fmt = "png";
  int captcha_add_tile_pattern = 1;
  int captcha_rotate = 1;
  for (x = 0; x < xmax; x++) {
    rpos = random() % nwords;
    word = words[rpos];

    blob = captcha_draw(magick_wand, d_wand, p_wand, word, captcha_file_fmt, captcha_add_tile_pattern, captcha_rotate, &blob_len);

    if (xmax == 1) {
      fprintf(stderr, "rpos=%d, word=%s\n", rpos, word);
      fprintf(stderr, "blob_len=%lu\n", blob_len);
      fd = open(captcha_file, O_CREAT|O_WRONLY|O_TRUNC, 0644);
      nwritten = write(fd, blob, blob_len);
      if (nwritten != blob_len) {
	fprintf(stderr, "Less data written to file.\n");
      }
      fprintf(stderr, "Image written to file: %s\n", captcha_file);
      close(fd);
    }

    MagickRelinquishMemory(blob);

    tpos = random() % CM;
    strncpy(choices[tpos], word, CN);

    for (i = 0; i < CM; i++) {
      if (i != tpos) {
	while ((tpos1 = (random() % nwords)) == rpos) { }
	word = words[tpos1];
	strncpy(choices[i], word, CN);
      }
      if (xmax == 1) {
	fprintf(stderr, "%d. %s\n", i, choices[i]);
      }
    }
  }

  /* Clean up */
  if (magick_wand) magick_wand = DestroyMagickWand(magick_wand);
  if (d_wand) d_wand = DestroyDrawingWand(d_wand);
  if (p_wand) p_wand = DestroyPixelWand(p_wand);

  MagickWandTerminus();

err:


  return 0;
}
