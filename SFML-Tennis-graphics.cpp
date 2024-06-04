////////////////////////////////////////////////////////////
// Header files
////////////////////////////////////////////////////////////
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <sstream> // 추가된 부분: 문자열 스트림을 사용하기 위한 헤더

#ifdef SFML_SYSTEM_IOS
#include <SFML/Main.hpp>
#endif

// Function to return the resource directory depending on the platform
std::string resourcesDir()
{
#ifdef SFML_SYSTEM_IOS
    return "";
#else
    return "resources/";
#endif
}

////////////////////////////////////////////////////////////
/// Entry point of the application
///
/// \return Exit code of the application
////////////////////////////////////////////////////////////
int main()
{
    std::srand(static_cast<unsigned int>(std::time(NULL)));

    // Constants definition
    const float pi = 3.14159f;
    const float gameWidth = 800;
    const float gameHeight = 600;
    sf::Vector2f paddleSize(25, 100);
    float ballRadius = 10.f;

    // Creating the application window
    sf::RenderWindow window(sf::VideoMode(static_cast<unsigned int>(gameWidth), static_cast<unsigned int>(gameHeight), 32), "SFML Tennis",
        sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(true);

    // Loading the sound for the game
    sf::SoundBuffer ballSoundBuffer;
    if (!ballSoundBuffer.loadFromFile(resourcesDir() + "ball.wav"))
        return EXIT_FAILURE;
    sf::Sound ballSound(ballSoundBuffer);

    // Creating the SFML logo texture
    sf::Texture sfmlLogoTexture;
    if (!sfmlLogoTexture.loadFromFile(resourcesDir() + "sfml_logo.png"))
        return EXIT_FAILURE;
    sf::Sprite sfmlLogo;
    sfmlLogo.setTexture(sfmlLogoTexture);
    sfmlLogo.setPosition(170, 50);

    // Creating the left paddle
    sf::RectangleShape leftPaddle;
    leftPaddle.setSize(paddleSize - sf::Vector2f(3, 3));
    leftPaddle.setOutlineThickness(3);
    leftPaddle.setOutlineColor(sf::Color::Black);
    leftPaddle.setFillColor(sf::Color(100, 100, 200));
    leftPaddle.setOrigin(paddleSize / 2.f);

    // Creating the right paddle
    sf::RectangleShape rightPaddle;
    rightPaddle.setSize(paddleSize - sf::Vector2f(3, 3));
    rightPaddle.setOutlineThickness(3);
    rightPaddle.setOutlineColor(sf::Color::Black);
    rightPaddle.setFillColor(sf::Color(200, 100, 100));
    rightPaddle.setOrigin(paddleSize / 2.f);

    // Creating the ball
    sf::CircleShape ball;
    ball.setRadius(ballRadius - 3);
    ball.setOutlineThickness(2);
    ball.setOutlineColor(sf::Color::Black);
    ball.setFillColor(sf::Color::White);
    ball.setOrigin(ballRadius / 2, ballRadius / 2);

    // Loading the font for the text
    sf::Font font;
    if (!font.loadFromFile(resourcesDir() + "tuffy.ttf"))
        return EXIT_FAILURE;

    // Initializing the pause message
    sf::Text pauseMessage;
    pauseMessage.setFont(font);
    pauseMessage.setCharacterSize(40);
    pauseMessage.setPosition(170.f, 200.f);
    pauseMessage.setFillColor(sf::Color::White);

    // Initializing the score text
    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(30);
    scoreText.setPosition(gameWidth / 2 - 50, 10);
    scoreText.setFillColor(sf::Color::White);

    // 추가된 부분: 플레이 시간을 표시할 텍스트 객체
    sf::Text playTimeText;
    playTimeText.setFont(font);
    playTimeText.setCharacterSize(20);
    playTimeText.setPosition(gameWidth / 2 - 50, gameHeight - 30); // 중앙 하단에 표시
    playTimeText.setFillColor(sf::Color::White);

#ifdef SFML_SYSTEM_IOS
    pauseMessage.setString("Welcome to SFML Tennis!\nTouch the screen to start the game.");
#else
    pauseMessage.setString("Welcome to SFML Tennis!\n\nPress the space key to start the game.");
#endif

    // Definition of paddle attributes
    sf::Clock AITimer;
    const sf::Time AITime = sf::seconds(0.1f);
    const float paddleSpeed = 400.f;
    float rightPaddleSpeed = 0.f;
    const float ballSpeed = 400.f;
    float ballAngle = 0.f; // To be changed later

    // Initializing score variables
    int playerScore = 0;
    int aiScore = 0;

    sf::Clock clock;
    bool isPlaying = false;

    sf::Clock playTimer; // 추가된 부분: 플레이 시간을 계산할 시계 객체

    while (window.isOpen())
    {
        // Event handling
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Close the window or exit when ESC is pressed
            if ((event.type == sf::Event::Closed) ||
                ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape)))
            {
                window.close();
                break;
            }

            // Start the game when the space key is pressed
            if (((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Space)) ||
                (event.type == sf::Event::TouchBegan))
            {
                if (!isPlaying)
                {
                    // (Re)start the game
                    isPlaying = true;
                    clock.restart();

                    // 추가된 부분: 플레이 시간 시계 초기화
                    playTimer.restart();

                    // Reset paddle and ball positions
                    leftPaddle.setPosition(10.f + paddleSize.x / 2.f, gameHeight / 2.f);
                    rightPaddle.setPosition(gameWidth - 10.f - paddleSize.x / 2.f, gameHeight / 2.f);
                    ball.setPosition(gameWidth / 2.f, gameHeight / 2.f);

                    // Initialize ball angle
                    do
                    {
                        // Adjust the initial angle of the ball to not be too vertical
                        ballAngle = static_cast<float>(std::rand() % 360) * 2.f * pi / 360.f;
                    } while (std::abs(std::cos(ballAngle)) < 0.7f);
                }
            }

            // Adjust the view when the window is resized
            if (event.type == sf::Event::Resized)
            {
                sf::View view;
                view.setSize(gameWidth, gameHeight);
                view.setCenter(gameWidth / 2.f, gameHeight / 2.f);
                window.setView(view);
            }
        }

        if (isPlaying)
        {
            float deltaTime = clock.restart().asSeconds();

            // Move the player's paddle
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) &&
                (leftPaddle.getPosition().y - paddleSize.y / 2 > 5.f))
            {
                leftPaddle.move(0.f, -paddleSpeed * deltaTime);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) &&
                (leftPaddle.getPosition().y + paddleSize.y / 2 < gameHeight - 5.f))
            {
                leftPaddle.move(0.f, paddleSpeed * deltaTime);
            }

            if (sf::Touch::isDown(0))
            {
                sf::Vector2i pos = sf::Touch::getPosition(0);
                sf::Vector2f mappedPos = window.mapPixelToCoords(pos);
                leftPaddle.setPosition(leftPaddle.getPosition().x, mappedPos.y);
            }

            // Move the computer's paddle
            if (((rightPaddleSpeed < 0.f) && (rightPaddle.getPosition().y - paddleSize.y / 2 > 5.f)) ||
                ((rightPaddleSpeed > 0.f) && (rightPaddle.getPosition().y + paddleSize.y / 2 < gameHeight - 5.f)))
            {
                rightPaddle.move(0.f, rightPaddleSpeed * deltaTime);
            }

            // Update the direction of the computer's paddle
            if (AITimer.getElapsedTime() > AITime)
            {
                AITimer.restart();
                if (ball.getPosition().y + ballRadius > rightPaddle.getPosition().y + paddleSize.y / 2)
                    rightPaddleSpeed = paddleSpeed;
                else if (ball.getPosition().y - ballRadius < rightPaddle.getPosition().y - paddleSize.y / 2)
                    rightPaddleSpeed = -paddleSpeed;
                else
                    rightPaddleSpeed = 0.f;
            }

            // Move the ball
            float factor = ballSpeed * deltaTime;
            ball.move(std::cos(ballAngle) * factor, std::sin(ballAngle) * factor);

            // 게임 플레이 시간을 계산
            sf::Time playTime = playTimer.getElapsedTime();

            // 플레이 시간을 문자열로 변환하여 텍스트에 설정
            std::ostringstream ss;
            ss << "Play Time: " << static_cast<int>(playTime.asSeconds()) << "s"; // 플레이 시간을 초 단위로 표시
            playTimeText.setString(ss.str());

            // Check for collisions between the ball and the screen
            if (ball.getPosition().x - ballRadius < 0.f)
            {
                isPlaying = false;
                aiScore++;
                pauseMessage.setString("You lost!\n\nPress the space key to restart\nor the ESC key to exit.");
            }
            if (ball.getPosition().x + ballRadius > gameWidth)
            {
                isPlaying = false;
                playerScore++;
                pauseMessage.setString("You won!\n\nPress the space key to restart\nor the ESC key to exit.");
            }
            if (ball.getPosition().y - ballRadius < 0.f)
            {
                ballSound.play();
                ballAngle = -ballAngle;
                ball.setPosition(ball.getPosition().x, ballRadius + 0.1f);
            }
            if (ball.getPosition().y + ballRadius > gameHeight)
            {
                ballSound.play();
                ballAngle = -ballAngle;
                ball.setPosition(ball.getPosition().x, gameHeight - ballRadius - 0.1f);
            }

            // Check for collisions between the ball and paddles
            // Left paddle
            if (ball.getPosition().x - ballRadius < leftPaddle.getPosition().x + paddleSize.x / 2 &&
                ball.getPosition().x - ballRadius > leftPaddle.getPosition().x &&
                ball.getPosition().y + ballRadius >= leftPaddle.getPosition().y - paddleSize.y / 2 &&
                ball.getPosition().y - ballRadius <= leftPaddle.getPosition().y + paddleSize.y / 2)
            {
                if (ball.getPosition().y > leftPaddle.getPosition().y)
                    ballAngle = pi - ballAngle + static_cast<float>(std::rand() % 20) * pi / 180;
                else
                    ballAngle = pi - ballAngle - static_cast<float>(std::rand() % 20) * pi / 180;

                ballSound.play();
                ball.setPosition(leftPaddle.getPosition().x + ballRadius + paddleSize.x / 2 + 0.1f, ball.getPosition().y);
            }

            // Right paddle
            if (ball.getPosition().x + ballRadius > rightPaddle.getPosition().x - paddleSize.x / 2 &&
                ball.getPosition().x + ballRadius < rightPaddle.getPosition().x &&
                ball.getPosition().y + ballRadius >= rightPaddle.getPosition().y - paddleSize.y / 2 &&
                ball.getPosition().y - ballRadius <= rightPaddle.getPosition().y + paddleSize.y / 2)
            {
                if (ball.getPosition().y > rightPaddle.getPosition().y)
                    ballAngle = pi - ballAngle + static_cast<float>(std::rand() % 20) * pi / 180;
                else
                    ballAngle = pi - ballAngle - static_cast<float>(std::rand() % 20) * pi / 180;

                ballSound.play();
                ball.setPosition(rightPaddle.getPosition().x - ballRadius - paddleSize.x / 2 - 0.1f, ball.getPosition().y);
            }

            // End the game when reaching 5 points
            if (playerScore >= 5 || aiScore >= 5)
            {
                isPlaying = false;
                if (playerScore >= 5)
                    pauseMessage.setString("Game over!\nYou won the match!\n\nPress the space key to restart\nor the ESC key to exit.");
                else
                    pauseMessage.setString("Game over!\nThe computer won the match!\n\nPress the space key to restart\nor the ESC key to exit.");
            }

            // Update the score text
            scoreText.setString("Player: " + std::to_string(playerScore) + " - AI: " + std::to_string(aiScore));
        }

        // Clear the window
        window.clear(sf::Color(50, 50, 50));

        if (isPlaying)
        {
            // Draw the paddles and the ball
            window.draw(leftPaddle);
            window.draw(rightPaddle);
            window.draw(ball);
        }
        else
        {
            // Draw the pause message
            window.draw(pauseMessage);
            window.draw(sfmlLogo);
        }

        // 추가된 부분: 플레이 시간 텍스트를 그림
        window.draw(playTimeText);

        // Display the score
        window.draw(scoreText);

        // Display the window
        window.display();
    }

    return EXIT_SUCCESS;
}

            
