const playButton = document.getElementById("playButton");

playButton.addEventListener('click', () => {
	document.body.classList.add("game-started");

	startGame();
})
