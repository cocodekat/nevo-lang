// COCO — system schematics
// lightbox + page furniture

function openLightbox(imgEl) {
  if (!imgEl.getAttribute('src')) return;
  const lightbox = document.getElementById('lightbox');
  const lightboxImg = document.getElementById('lightbox-img');
  lightboxImg.src = imgEl.src;
  lightboxImg.alt = imgEl.alt;
  lightbox.classList.add('open');
  document.body.style.overflow = 'hidden';
}

function closeLightbox() {
  const lightbox = document.getElementById('lightbox');
  if (!lightbox) return;
  lightbox.classList.remove('open');
  document.body.style.overflow = '';
}

document.addEventListener('keydown', (e) => {
  if (e.key === 'Escape') closeLightbox();
});

// corner registration marks + grid field, injected so every page gets them
function injectPageFurniture() {
  const grid = document.createElement('div');
  grid.className = 'gridfield';
  grid.setAttribute('aria-hidden', 'true');
  document.body.prepend(grid);

  ['tl', 'tr', 'bl', 'br'].forEach((pos) => {
    const mark = document.createElement('div');
    mark.className = `reg-mark ${pos}`;
    mark.setAttribute('aria-hidden', 'true');
    document.body.appendChild(mark);
  });
}

injectPageFurniture();

// draw the sheet frame from a single origin dot, then reveal content
function initSheetFrame() {
  const frame = document.querySelector('.sheet-border');
  if (!frame) return;

  const contentEls = Array.from(frame.querySelectorAll(
    ':scope > .sheet-topstrip, :scope > .sheet-body > *, :scope > .titleblock'
  ));
  contentEls.forEach((el) => el.classList.add('fade-el'));

  const reduceMotion = window.matchMedia('(prefers-reduced-motion: reduce)').matches;
  if (reduceMotion) {
    contentEls.forEach((el) => el.classList.add('visible'));
    return;
  }

  const rect = frame.getBoundingClientRect();
  const w = Math.round(rect.width);
  const h = Math.round(rect.height);
  if (w < 10 || h < 10) {
    contentEls.forEach((el) => el.classList.add('visible'));
    return;
  }

  frame.classList.add('framing');

  const svgNS = 'http://www.w3.org/2000/svg';
  const inset = 0.75;

  const svg = document.createElementNS(svgNS, 'svg');
  svg.setAttribute('class', 'frame-svg');
  svg.setAttribute('viewBox', `0 0 ${w} ${h}`);
  svg.setAttribute('preserveAspectRatio', 'none');
  svg.setAttribute('aria-hidden', 'true');

  const rectEl = document.createElementNS(svgNS, 'rect');
  rectEl.setAttribute('class', 'frame-rect');
  rectEl.setAttribute('x', inset);
  rectEl.setAttribute('y', inset);
  rectEl.setAttribute('width', Math.max(w - inset * 2, 1));
  rectEl.setAttribute('height', Math.max(h - inset * 2, 1));

  const perimeter = 2 * (w + h);
  rectEl.style.strokeDasharray = String(perimeter);
  rectEl.style.strokeDashoffset = String(perimeter);

  const dot = document.createElementNS(svgNS, 'circle');
  dot.setAttribute('class', 'frame-dot');
  dot.setAttribute('cx', inset);
  dot.setAttribute('cy', inset);
  dot.setAttribute('r', 3);

  svg.appendChild(rectEl);
  svg.appendChild(dot);
  frame.prepend(svg);

  // keep border draw speed roughly constant regardless of page length
  const duration = Math.min(3400, Math.max(1100, perimeter / 2.3));

  requestAnimationFrame(() => {
    requestAnimationFrame(() => {
      dot.classList.add('pop');
      rectEl.style.transition = `stroke-dashoffset ${duration}ms cubic-bezier(.6,0,.3,1)`;
      rectEl.style.transitionDelay = '150ms';
      rectEl.style.strokeDashoffset = '0';
    });
  });

  const revealStart = duration + 150 + 80;

  contentEls.forEach((el, i) => {
    setTimeout(() => {
      el.classList.add('visible');
      if (el.classList.contains('schematic')) el.classList.add('armed');
    }, revealStart + i * 55);
  });

  setTimeout(() => frame.classList.add('framed'), revealStart - 100);

  // keep the frame sized correctly on resize without re-triggering the draw
  let resizeTimer;
  window.addEventListener('resize', () => {
    clearTimeout(resizeTimer);
    resizeTimer = setTimeout(() => {
      const r = frame.getBoundingClientRect();
      const nw = Math.round(r.width);
      const nh = Math.round(r.height);
      svg.setAttribute('viewBox', `0 0 ${nw} ${nh}`);
      rectEl.setAttribute('width', Math.max(nw - inset * 2, 1));
      rectEl.setAttribute('height', Math.max(nh - inset * 2, 1));
    }, 150);
  });
}

initSheetFrame();
