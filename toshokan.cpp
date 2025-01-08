#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <mupdf/fitz.h>

#define CLAY_IMPLEMENTATION
#include "clay.h"

const int FONT_ID_BODY_16 = 0;
const int screenWidth = 1920;
const int screenHeight = 1080;

const Clay_Color COLOR_LIGHT = { 224, 215, 210, 255 };
const Clay_Color COLOR_RED = { 168, 66, 28, 255 };
const Clay_Color COLOR_ORANGE = { 225, 138, 50, 255 };

static void errorHandler(Clay_ErrorData errorData) {
	fprintf(stderr, "Clay Error Message: %s\n", errorData.errorText.chars);
}


typedef struct
{
	uint32_t fontId;
	TTF_Font* font;
} SDL2_Font;


static SDL2_Font SDL3_fonts[1];

static Clay_Dimensions SDL3_MeasureText(Clay_String *text, Clay_TextElementConfig* config)
{
	TTF_Font* font = SDL3_fonts[config->fontId].font;
	char* chars = (char*)calloc(text->length + 1, 1);
	memcpy(chars, text->chars, text->length);
	int width, height;
	if (TTF_GetStringSize(font, chars,text->length, &width, &height) < 0) {
		fprintf(stderr, "TTF_GetStringSize Error: %s\n", SDL_GetError());
	}
	free(chars);
	return { 
		.width = static_cast<float>(width),
		.height = static_cast<float>(height)
	};
}

SDL_Rect currentClippingRectangle;

static void Clay_SDL2_Render(SDL_Renderer* renderer, Clay_RenderCommandArray renderCommands)
{
	for (uint32_t i = 0; i < renderCommands.length; i++)
	{
		Clay_RenderCommand* renderCommand = Clay_RenderCommandArray_Get(&renderCommands, i);
		Clay_BoundingBox boundingBox = renderCommand->boundingBox;
		switch (renderCommand->commandType)
		{
		case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
			Clay_RectangleElementConfig* config = renderCommand->config.rectangleElementConfig;
			Clay_Color color = config->color;
			SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
			SDL_FRect rect = {
					.x = boundingBox.x,
					.y = boundingBox.y,
					.w = boundingBox.width,
					.h = boundingBox.height,
			};
			SDL_RenderFillRect(renderer, &rect);
			break;
		}
		case CLAY_RENDER_COMMAND_TYPE_TEXT: {
			Clay_TextElementConfig* config = renderCommand->config.textElementConfig;
			Clay_String text = renderCommand->text;
			char* cloned = (char*)calloc(text.length + 1, 1);
			memcpy(cloned, text.chars, text.length);
			TTF_Font* font = SDL3_fonts[config->fontId].font;
			SDL_Color sdl_color = {
					.r = static_cast<Uint8>(config->textColor.r),
					.g = static_cast<Uint8>(config->textColor.g),
					.b = static_cast<Uint8>(config->textColor.b),
					.a = static_cast<Uint8>(config->textColor.a),
			};

			SDL_Surface* surface = TTF_RenderText_Solid (font, cloned, text.length, sdl_color);
			SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

			SDL_FRect destination = {
					.x = boundingBox.x,
					.y = boundingBox.y,
					.w = boundingBox.width,
					.h = boundingBox.height,
			};
			SDL_RenderTexture (renderer, texture, NULL, &destination);

			SDL_DestroyTexture(texture);
			SDL_DestroySurface(surface);
			free(cloned);
			break;
		}
		case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START: {
			currentClippingRectangle = {
				.x = static_cast<int>(boundingBox.x),
				.y = static_cast<int>(boundingBox.y),
				.w = static_cast<int>(boundingBox.width),
				.h = static_cast<int>(boundingBox.height),
			};
			SDL_SetRenderClipRect(renderer, &currentClippingRectangle);
			break;
		}
		case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END: {
			SDL_SetRenderClipRect(renderer, NULL);
			break;
		}
		default: {
			fprintf(stderr, "Error: unhandled render command: %d\n", renderCommand->commandType);
			exit(1);
		}
		}
	}
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

	if (TTF_Init() < 0) {
		fprintf(stderr, "TTF_Init Error: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}
	TTF_Font* font = TTF_OpenFont("resources/Roboto-Regular.ttf", 16);
	if (!font) {
		fprintf(stderr, "TTF_OpenFont Error: %s\n", SDL_GetError());
		TTF_Quit();
		SDL_Quit();
		return 1;
	}

	SDL3_fonts[FONT_ID_BODY_16] = {
		.fontId = FONT_ID_BODY_16,
		.font = TTF_OpenFont("fonts/Roboto-Regular.ttf", 16),
	};

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
	Clay_SetMeasureTextFunction(SDL3_MeasureText);
	int windowWidth = 0;
	int windowHeight = 0;
	SDL_GetWindowSize(window, &windowWidth, &windowHeight);
	Clay_Initialize(arena, { screenWidth , screenHeight }, { errorHandler });
	Uint64 NOW = SDL_GetPerformanceCounter();
	Uint64 LAST = 0;
	double deltaTime = 0;

	
	SDL_Event event;
	int quit = 0;
	while (!quit)
	{
		Clay_Vector2 scrollDelta = {};
		
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
			{
				quit = 1;
			}
		}
		Clay_RenderCommandArray CreateLayout();
		Clay_BeginLayout();

		CLAY(CLAY_LAYOUT({ .padding = 8 })) {
			// Child element 1
			CLAY_TEXT(CLAY_STRING("Hello World"), CLAY_TEXT_CONFIG({ .fontSize = 16 }));
			// Child element 2 with red background
			CLAY(CLAY_RECTANGLE({ .color = COLOR_RED })) {
				// etc
			}
		}

		Clay_RenderCommandArray renderCommands = Clay_EndLayout();



		SDL_GetWindowSize(window, &windowWidth, &windowHeight);
		Clay_SetLayoutDimensions( { (float)windowWidth, (float)windowHeight });

		SDL_RenderClear(renderer);

		Clay_SDL2_Render(renderer, renderCommands);

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









