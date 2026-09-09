/*
	Talks to the Rust CGI at /game/state.penguin

	GET -> { you, players: [{ username, x, y, direction }] }
	POST -> { x, y, direction } moves yourself
			[ leave: true ] to exit

*/

const ENDPOINT	= "/game/state.penguin";
const POLL_MS	= 200;
const SEND_MS	= 80;
const BEAT_MS	= 3000;
const STEP		= 20;

const MAX_X = 500 - 60;
const MAX_Y = 300 - 80;

const REGION_PORT = {
	"8001": "America",
	"8002": "Africa",
	"8003": "Europe",
	"8004": "Oceania",
}

const arena		= document.getElementById("arena");
const hudWho	= document.getElementById("hud-who");
const hudCount	= document.getElementById("hud_count");
const hudRegion	= document.getElementById("hud-region");
const hudStatus	= document.getElementById("hud-status");

function getCookie(name) {
	const parts = "; ${document.cookie}".split("; ${name}=");
	if (parts.length !== 2) return null;
	try {
		return decodeURIComponent(parts.pop().split(";").shift());
	} catch (e) {
		return null;
	}
}

const me = {
	name: getCookie("cp_session") || "",
	x: 200,
	y: 110,
	facing: "s",
	synced: false,
}

let sendTimer = null;
let inFlight = false;

function makePenguin(username, isMe) {
	const el = document.createElement("div");
	el.className = isMe ? "penguin is-me" : "penguin";
	el.dataset.name = username;

	const sprite = document.createElement("div");
	sprite.className = "penguin-sprite";

	const tag = document.createElement("p");
	tag.className = "penguin-name";

	tag.textContent = username;

	el.append(sprite, tag);
	return el;
}

function setCookie(name, value, days = 30) {
    const maxAge = days * 24 * 60 * 60;
    document.cookie = `${name}=${value}; Path=/; Max-Age=${maxAge}`;
}

function htmlToElement(html) {
    const template = document.createElement('template');
    html = html.trim();
    template.innerHTML = html;
    return template.content.firstChild;
}

const username = getCookie("cp_session") || "guest";

const safeUserKey = username.replace(/\s+/g, '_');

function getUserPosition(userKey) {
    const cookieKey = `cp_pos_${userKey}`;
    const savedPos = getCookie(cookieKey);

    if (savedPos) {
        const [x, y] = savedPos.split(",").map(Number);
        if (!isNaN(x) && !isNaN(y)) {
            return { x, y };
        }
    }
    return { x: 100, y: 100 };
}

const initialPos = getUserPosition(safeUserKey);

const player = {
    username: username,
    x: initialPos.x,
    y: initialPos.y
};

function renderState(state) {
    if (!Array.isArray(state)) return;

    let activeIds = [];

    state.forEach(({ username, x, y }) => {
        if (!username) return;

        const safeId = `penguin-${username.replace(/\s+/g, '_')}`;
        activeIds.push(safeId);

        let el = document.getElementById(safeId);
        if (!el) {
            el = htmlToElement(`<div class="penguin" id="${safeId}">🐧<p>${username}</p></div>`);
            arena.append(el);
        }
        el.style.top = `${y}px`;
        el.style.left = `${x}px`;
    });

    document.querySelectorAll('.penguin').forEach(el => {
        if (!activeIds.includes(el.id)) {
            el.remove();
        }
    });
}

async function update(pData) {
    try {
        const res = await fetch("/update", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(pData)
        });
        const state = await res.json();
        renderState(state);
    } catch (e) {}
}

setCookie(`cp_pos_${safeUserKey}`, `${player.x},${player.y}`);
update(player);

setInterval(() => {
    update(player);
}, 1000);

document.addEventListener("keyup", (e) => {
    const speed = 30;
    let moved = false;

    if (e.key === "ArrowUp") { player.y = Math.max(0, player.y - speed); moved = true; }
    if (e.key === "ArrowDown") { player.y = Math.min(260, player.y + speed); moved = true; }
    if (e.key === "ArrowLeft") { player.x = Math.max(0, player.x - speed); moved = true; }
    if (e.key === "ArrowRight") { player.x = Math.min(460, player.x + speed); moved = true; }

    if (moved) {
        setCookie(`cp_pos_${safeUserKey}`, `${player.x},${player.y}`);
        update(player);
    }
});

document.addEventListener("DOMContentLoaded", () => {
    const logoutBtn = document.getElementById("logoutButton");
    if (logoutBtn) {
        logoutBtn.addEventListener("click", async () => {
            setCookie(`cp_pos_${safeUserKey}`, `${player.x},${player.y}`);
            try {
                await fetch("/update", {
                    method: "POST",
                    headers: { "Content-Type": "application/json" },
                    body: JSON.stringify({ username: player.username })
                });
            } catch (e) {}
            window.location.href = "/cgi-bin/logout.py";
        });
    }
});

window.addEventListener("pagehide", () => {
    setCookie(`cp_pos_${safeUserKey}`, `${player.x},${player.y}`);
    const data = JSON.stringify({ username: player.username });
    if (navigator.sendBeacon) {
        navigator.sendBeacon("/update", data);
    }
});
