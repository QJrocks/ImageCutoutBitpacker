//This program is an example on how to use the shader included to play back the packed images.
//While this program is pretty heavily reliant on SFML, the heart of it is the GLSL shader, which can be implemented in any way you need.
#include <SFML/Graphics.hpp>

int main() {    
    //Basic SFML skeleton, setting up window
    sf::RenderWindow m_window;
    //Try for a 1920x1080 window, but shrink it if the user's monitor is smaller
    sf::Vector2u windowSize(std::min(1920u, sf::VideoMode::getDesktopMode().width), std::min(1080u, sf::VideoMode::getDesktopMode().height));
    m_window.create(sf::VideoMode(windowSize.x, windowSize.y), "Bitpacked Image Playback Demo", sf::Style::Default);
    m_window.setFramerateLimit(60);

    sf::Event m_event;

    //this should match the settings used to pack the image
    const int bitsPerLayer = 2;

    //Create shader and set up uniforms
    sf::Shader playbackShader;
    playbackShader.loadFromFile("bitpacked_image_player.frag", sf::Shader::Fragment);
    playbackShader.setUniform("texture", sf::Shader::CurrentTexture);
    playbackShader.setUniform("bitsPerLayer", bitsPerLayer);

    //6 images used in the sample
    const int textureCount = 6;
    sf::Texture textureArray[textureCount];

    //Load up the textures
    for (int x = 0; x < textureCount; x++) {
        textureArray[x].loadFromFile("Images/output_" + std::to_string(x) + ".png");
    }

    //Basic sprite to display the texture
    sf::Sprite displaySprite;
    displaySprite.setTexture(textureArray[0]);
    displaySprite.setTextureRect(sf::IntRect(0, 0, 1920, 1080));

    //Variables for handling playback
    int currentFrame = 0;
    int currentTexture = 0;
    const int totalFrameCount = 90;

    //Main loop
    while (m_window.isOpen()) {
        //Mandatory event handling
        while (m_window.pollEvent(m_event)) {
            switch (m_event.type) {
            case sf::Event::Closed:
                m_window.close();
                break;
            //If user presses Esc, close window
            case sf::Event::KeyPressed:
                if (m_event.key.code == sf::Keyboard::Escape) {
                    m_window.close();
                }
                break;
            }
        }

        //Send the current layer to the shader 
        int currentLayer = currentFrame % (32 / bitsPerLayer);
        playbackShader.setUniform("currentLayer", currentLayer);

        //If the calculated next texture is different from the current one (aka, this layer is the first layer of a new texture), swap out the current texture for the next one.
        int nextTexture = currentFrame / (32 / bitsPerLayer);
        if (currentTexture != nextTexture) {
            currentTexture = nextTexture;
            displaySprite.setTexture(textureArray[currentTexture]);
        }

        //Increment and loop the current frame
        currentFrame = (currentFrame + 1) % totalFrameCount;

        //Basic rendering code
        m_window.clear(sf::Color::White);
        m_window.draw(displaySprite, &playbackShader);
        m_window.display();
    }
}

//windows entrypoint so that the program doesn't launch with a useless console window
#include <windows.h>
int APIENTRY WinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE hInstPrev, _In_ PSTR cmdline, _In_ int cmdshow) {
    return main();
}