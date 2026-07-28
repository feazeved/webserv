document.getElementById('playbutton').addEventListener('click', () => {
	document.body.classList.add('game-started');

	document.getElementById('playbutton').style.display = 'none';

	const canvas = document.getElementById('gameCanvas');
	canvas.style.display = 'block';

	startGame();
})
