#include <SFML/Graphics.hpp>
extern "C"
{
  #include <libavformat/avformat.h>
  #include <libavcodec/avcodec.h>
}

sf::RenderWindow App;
sf::Image im_video;
sf::Sprite sp_video;
sf::Uint8 *Data;

AVFormatContext *pFormatCtx;
int             videoStream;
int             iFrameSize;
AVCodecContext  *pCodecCtx;
AVFrame         *pFrame;
AVFrame         *pFrameRGB;
uint8_t         *buffer;
AVPacket        packet;

int init_video(char* filename);
void display();
void close_video();


int init_video(char* filename)
{

	AVCodec   *pCodec;

	av_register_all();

  if(av_open_input_file(&pFormatCtx, filename, NULL, 0, NULL)!=0)
  {
    fprintf(stderr, "Unexisting file!\n");
    return -1;
  }

  if(av_find_stream_info(pFormatCtx)<0)
  {
    fprintf(stderr, "Couldn't find stream information!\n");
    return -1;
  }

  dump_format(pFormatCtx, 0, filename, 0);

  videoStream=-1;
  for(int i=0; i<(pFormatCtx->nb_streams); i++)
  {
    if(pFormatCtx->streams[i]->codec.codec_type==CODEC_TYPE_VIDEO)
    {
      videoStream=i;
      break;
    }
  }
  if(videoStream==-1)
    return -1;

  pCodecCtx=&pFormatCtx->streams[videoStream]->codec;

  pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
  if(pCodec==NULL)
  {
    fprintf(stderr, "Unsupported codec!\n");
    return -1;
  }

  if(avcodec_open(pCodecCtx, pCodec)<0)
    return -1;
  iFrameSize = pCodecCtx->width * pCodecCtx->height * 3;

  pFrame=avcodec_alloc_frame();

  pFrameRGB=avcodec_alloc_frame();
  if(pFrameRGB==NULL)
    return -1;

  int numBytes=avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
  buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

  avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24,
                pCodecCtx->width, pCodecCtx->height);

  Data = new sf::Uint8[pCodecCtx->width * pCodecCtx->height * 4];

  return 0;
}

void display()
{
  int frameFinished;

  if (av_read_packet(pFormatCtx, &packet) < 0)
  {
    close_video();
    //exit(0);
  }

  if(packet.stream_index==videoStream)
  {
	avcodec_decode_video(pCodecCtx, pFrame, &frameFinished,
                        packet.data, packet.size);

    if(frameFinished)
    {
      // Convert the image from its native format to RGB
      img_convert((AVPicture *)pFrameRGB, PIX_FMT_RGB24,
                  (AVPicture*)pFrame, pCodecCtx->pix_fmt, pCodecCtx->width,
                  pCodecCtx->height);
	  }

	int j = 0;
    for(int i = 0 ; i < (iFrameSize) ; i+=3)
    {
      Data[j] = pFrameRGB->data[0][i];
      Data[j+1] = pFrameRGB->data[0][i+1];
      Data[j+2] = pFrameRGB->data[0][i+2];
      Data[j+3] = 255;
      j+=4;
    }
    im_video.LoadFromPixels(pCodecCtx->width, pCodecCtx->height, Data);

	}
  // Dessiner l'image sur le tampon de l'écran
  App.Draw(sp_video);
}

void close_video()
{
  // Libérer le packet alloué par av_read_frame
  av_free_packet(&packet);
  // Libérer l'image RGB
  av_free(buffer);
  av_free(pFrameRGB);
  // Libérer l'image YUV
  av_free(pFrame);
  // Fermer le codec
  avcodec_close(pCodecCtx);
  // Fermer le  fichier video
  av_close_input_file(pFormatCtx);
}

int main()
{
	// Notre fonction pour initialiser la video
	if ( init_video("test.avi") == 0 )
	{
	  // Code SFML de base
	  App.Create( sf::VideoMode(pCodecCtx->width*2, pCodecCtx->height*2, 32),
	                              "Video avec SFML et FFMpeg"
	                            );

		// On crée notre image, en blanc par exemple
	  im_video.Create(pCodecCtx->width, pCodecCtx->height, sf::Color(255,255,255,255));
		// J'aime bien ne pas mettre le smooth, ça dépend de la qualité de la video :)
	  im_video.SetSmooth(false);
		// On crée notre sprite
	  sp_video.SetImage(im_video);
		// Vous pouvez utiliser les fonctionnalité du sprite sur la video,
		// comme le scale, de la même manière qu'une simple image fixe

	  // La boucle principale
	  bool Running = true;
	  while (Running)
	  {
	    // Les évènements
	    sf::Event Event;
	    while (App.GetEvent(Event))
	    {
	      if (Event.Type == sf::Event::Closed)
	        Running = false;

	      if ((Event.Type == sf::Event::KeyPressed) && (Event.Key.Code == sf::Key::Escape))
	        Running = false;
	    }

			// Notre fonction de lecture et dessin de la video
	    display();

			// On affiche tout ça
	    App.Display();
	    App.SetFramerateLimit(50);
	  }

		// Notre fonction pour fermer la video
	  close_video();
	  return EXIT_SUCCESS;
	}
  return EXIT_FAILURE;
}