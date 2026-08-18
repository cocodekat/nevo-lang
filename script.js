function openLightbox(imgEl) {
  if (!imgEl.getAttribute('src')) return; // skip empty placeholders
  const lightbox = document.getElementById('lightbox');
  const lightboxImg = document.getElementById('lightbox-img');
  lightboxImg.src = imgEl.src;
  lightboxImg.alt = imgEl.alt;
  lightbox.classList.add('open');
  document.body.style.overflow = 'hidden';
}

function closeLightbox() {
  document.getElementById('lightbox').classList.remove('open');
  document.body.style.overflow = '';
}

document.addEventListener('keydown', (e) => {
  if (e.key === 'Escape') closeLightbox();
});

document.addEventListener('mousemove', (e) => {
  document.documentElement.style.setProperty('--mx', e.clientX + 'px');
  document.documentElement.style.setProperty('--my', e.clientY + 'px');
});

function initRevealToggle() {
  if (localStorage.getItem('reveal-off') === 'true') {
    document.documentElement.classList.add('reveal-off');
  }
  syncTogglePressedState();
}

function toggleReveal() {
  document.documentElement.classList.toggle('reveal-off');
  localStorage.setItem('reveal-off', document.documentElement.classList.contains('reveal-off'));
  syncTogglePressedState();
}

function syncTogglePressedState() {
  const btn = document.querySelector('.reveal-toggle');
  if (btn) btn.setAttribute('aria-pressed', !document.documentElement.classList.contains('reveal-off'));
}

initRevealToggle();

function initRevealBackground() {
  const bg = document.body.dataset.bg;
  if (!bg) return; // no override — falls back to background.png from the CSS
  const url = `url('${bg}')`;
  document.querySelectorAll('.code-reveal, .code-reveal-glow').forEach(el => {
    el.style.backgroundImage = url;
  });
}

initRevealBackground();