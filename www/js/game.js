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

function setStatus(message) {
	if (!hudStatus) return;
	hudStatus.textContent = message || "";
	hudStatus.classList.toggle("hud-warn", Boolean(message));
	hudStatus.style.display = message ? "" : "none";
}

function renderState(state) {
    if (!state || !Array.isArray(state.players)) return;

	const seen = new Set();

	for (const p of state.players) {
		if (!p || typeof p.username !== "string" || !p.username) continue;

		const isMe = p.username === state.you;
		seen.add(p.username);

		let el = arena.querySelector('[data-name="${CSS.escape(p.username)}"]');
		if (!el) {
			el = makePenguin(p.username, isMe);
			arena.append(el);
		}

		const x = isMe ? me.x : p.x;
		const y = isMe ? me.y : p.y;
		const facing = isMe ? me.facing : p.facing;

		el.style.left = "${x}px";
		el.style.top = "${y}px";
		el.dataset.facing = facing || "s";
		el.style.zIndex = String(100 + Math.round(y));
	}

	for (const el of arena.querySelectorAll(".penguin")) {
		if (!seen.has(el.dataset.name)) el.remove();
	}

	if (hudCount) {
		const n = state.players.length;
		hudCount.textContent = n === 1 ? "1 penguin online" : "${n} penguins online";
	}

	if (hudWho && state.you) hudWho.textContent = state.you;
	setStatus(null);
}

async function call(method, body) {
	const options = { method, cache: "no-store" };
	if (body !== undefined) {
		options.headers = { "Content-Type": "application/json" };
		options.body = JSON.stringify(body);
	}
	const res = await fetch(ENDPOINT, options);
	if (res.status === 401) {
		window.location.href = "/login.html";
		throw new Error("unauthorized");
	}
	if (!res.ok) throw new Error("HTTP ${res.status}");
	return res.json();
}

async function poll() {
	try {
		const state = await call("GET");
		if (!me.synced && state.you) {
			const mine = state.players.find((p) => p.username === state.you);

			if (mine) {
				me.x = mine.x;
				me.y = mine.y;
				me.facing = mine.facing || "s";
			}
			me.name = state.you;
			me.synced = true;
		}
		renderState(state);
	} catch (e) {
		if (e.message !== "unauthorized") setStatus("Reconnecting...");
	}
}

async function flushMove() {
	sendTimer = null;
	if (inFlight) {
		scheduleSend();
		return;
	}
	inFlight = true;
	try {
		renderState(await call("POST", { x: me.x, y: me.y, facing: me.facing }));
	} catch (e) {
		if (e.message !== "unauthorized") setStatus("Reconnecting...");
	} finally {
		inFlight = false;
	}
}

function scheduleSend() {
	if (sendTimer === null) sendTimer = setTimeout(flushMove, SEND_MS);
}

function leave() {
	const payload = JSON.stringify({ leave: true });

	if (navigator.sendBeacon) {
		navigator.sendBeacon(ENDPOINT, new Blob([payload], { type: "application/json" }));
	} else {
		fetch(ENDPOINT, { method: "POST", body: payload, keepalive: true }).catch(() => { });
	}
}

const KEYS = {
	ArrowUp:    ["n",  0, -1], w: ["n",  0, -1], W: ["n",  0, -1],
	ArrowDown:  ["s",  0,  1], s: ["s",  0,  1], S: ["s",  0,  1],
	ArrowLeft:  ["w", -1,  0], a: ["w", -1,  0], A: ["w", -1,  0],
	ArrowRight: ["e",  1,  0], d: ["e",  1,  0], D: ["e",  1,  0],
};

document.addEventListener("keydown", (event) => {
	if (event.ctrlKey || event.metaKey || event.altKey) return;

	const move = KEYS[event.key];
	if (!move) return;
	event.preventDefault();

	const [facing, dx, dy] = move;
	const nx = Math.min(MAX_X, Math.max(0, me.x + dx * STEP));
	const ny = Math.min(MAX_Y, Math.max(0, me.y + dy * STEP));

	if (nx === me.x && ny === me.y && facing === me.facing) return;

		me.x = nx;
		me.y = ny;
		me.facing = facing;

		const self = arena.querySelector(".penguin.is-me");
		if (self) {
			self.style.left = `${me.x}px`;
			self.style.top = `${me.y}px`;
			self.dataset.facing = me.facing;
			self.style.zIndex = String(100 + Math.round(me.y));
		}

		scheduleSend();
})

const logoutButton = document.getElementById("logoutButton");
if (logoutButton) {
	logoutButton.addEventListener("click", () => {
		leave();
		window.location.href = "/cgi-bin/logout.py";
	})
}

window.addEventListener("pagehide", leave);

if (hudRegion) {
	hudRegion.textContent = REGION_PORT[window.location.port] || "Local";
}

poll();
setInterval(poll, POLL_MS);
setInterval(scheduleSend(), BEAT_MS);
