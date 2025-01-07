#include <stdio.h>
#include <SDL3/SDL.h>

#include <mupdf/fitz.h>

#define CLAY_IMPLEMENTATION
#include "clay.h"

const int screenWidth = 1920;
const int screenHeight = 1080;

//void (*errorHandlerFunction)(Clay_ErrorData errorText);

void errorHandler(Clay_ErrorData errorData) {
	// Log the error details
	fprintf(stderr, "Clay Error:\n");
	fprintf(stderr, "  Message: %s\n", errorData.errorText.chars);
}

int main(int argc, char* argv[])
{
    char* input;
    float zoom, rotate;
	int page_number, page_count;
	fz_context* ctx;
	fz_document* doc;
	fz_pixmap* pix{};
	fz_matrix ctm;

	if (argc < 3)
	{
		fprintf(stderr, "usage: %s input zoom rotate page_number\n", argv[0]);
		return EXIT_FAILURE;
	}

	input = argv[1];
	page_number = atoi(argv[2]) - 1;
	zoom = argc > 3 ? atof(argv[3]) : 100;
	rotate = argc > 4 ? atof(argv[4]) : 0;

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

	ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
    if (!ctx) {
		fprintf(stderr, "cannot create mupdf context\n");
        SDL_Quit();
		return EXIT_FAILURE;
    }

    fz_try(ctx)
	fz_register_document_handlers(ctx);
    fz_catch(ctx)
    {
        fz_report_error(ctx);
		fprintf(stderr, "cannot register document handlers\n");
		fz_drop_context(ctx);
		SDL_Quit();
		return EXIT_FAILURE;
    }

	fz_try(ctx)
		doc = fz_open_document(ctx, input);
    fz_catch(ctx)
    {
        fz_report_error(ctx);
		fprintf(stderr, "cannot open document: %s\n", input);
		fz_drop_context(ctx);
		SDL_Quit();
		return EXIT_FAILURE;
    }

	fz_try(ctx)
		page_count = fz_count_pages(ctx, doc);
	fz_catch(ctx)
	{
		fz_report_error(ctx);
		fprintf(stderr, "cannot count pages\n");
		fz_drop_document(ctx, doc);
		fz_drop_context(ctx);
		SDL_Quit();
		return EXIT_FAILURE;
	}

	if (page_number < 0 || page_number > page_count)
	{
		fprintf(stderr, "page number out of range: %d\n", page_number);
		fz_drop_document(ctx, doc);
		fz_drop_context(ctx);
		SDL_Quit();
		return EXIT_FAILURE;
	}

	ctm = fz_scale(zoom/100, zoom/100);
	ctm = fz_pre_rotate(ctm, rotate);

	fz_try(ctx)
		pix = fz_new_pixmap_from_page_number(ctx, doc, page_number, ctm, fz_device_rgb(ctx), 0);
	fz_catch(ctx)
	{
		fz_report_error(ctx);
		fprintf(stderr, "cannot create pixmap for page %d\n", page_number);
		fz_drop_document(ctx, doc);
		fz_drop_context(ctx);
		SDL_Quit();
		return EXIT_FAILURE;
	}

	SDL_Window* window = SDL_CreateWindow("My Window",
		pix->w, pix->h, 
		SDL_WINDOW_RESIZABLE);

	if (!window)
	{
		fprintf(stderr, "Window Creation Error: %s\n", SDL_GetError());
		fz_drop_pixmap(ctx, pix);
		fz_drop_document(ctx, doc);
		fz_drop_context(ctx);
		SDL_Quit();
		return EXIT_FAILURE;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
	if (!renderer)
	{
		fprintf(stderr, "Renderer Creation Error: %s\n", SDL_GetError());
		SDL_DestroyWindow(window);
		fz_drop_pixmap(ctx, pix);
		fz_drop_document(ctx, doc);
		fz_drop_context(ctx);
		SDL_Quit();
		return EXIT_FAILURE;
	}

	SDL_Surface* surface = SDL_CreateSurfaceFrom( pix->w, pix->h, SDL_PIXELFORMAT_RGB24, pix->samples, pix->stride);
	if (!surface)
	{
		fprintf(stderr, "Surface Creation Error: %s\n", SDL_GetError());
		SDL_DestroyWindow(window);
		fz_drop_pixmap(ctx, pix);
		fz_drop_document(ctx, doc);
		fz_drop_context(ctx);
		SDL_Quit();
		return EXIT_FAILURE;
	}

	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	if (!texture)
	{
		fprintf(stderr, "Texture Creation Error: %s\n", SDL_GetError());
		SDL_DestroySurface(surface);
		SDL_DestroyWindow(window);
		fz_drop_pixmap(ctx, pix);
		fz_drop_document(ctx, doc);
		fz_drop_context(ctx);
		SDL_Quit();
		return EXIT_FAILURE;
	}

	uint64_t totalMemorySize = Clay_MinMemorySize();
	Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, malloc(totalMemorySize));
	Clay_ErrorHandler error { errorHandler };
	Clay_Initialize(arena, { screenWidth , screenHeight }, error);

	SDL_Event event;
	int quit = 0;
	while (!quit)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
			{
				quit = 1;
			}
		}

		SDL_RenderClear(renderer);
		SDL_RenderTexture(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
	}

	SDL_DestroyTexture(texture);
	SDL_DestroySurface(surface);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	fz_drop_pixmap(ctx, pix);
	fz_drop_document(ctx, doc);
	fz_drop_context(ctx);
    SDL_Quit();

    return EXIT_SUCCESS;
}









