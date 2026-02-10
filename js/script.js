// Mobile menu toggle
const hamburger = document.querySelector('.hamburger');
const header = document.querySelector('.navbar');
const mobileMenu = document.getElementById('mobile-menu');

if (hamburger && header && mobileMenu) {
  hamburger.addEventListener('click', () => {
    const isOpen = header.classList.toggle('mobile-open');
    hamburger.setAttribute('aria-expanded', String(isOpen));
  });

  mobileMenu.querySelectorAll('a').forEach(a => {
    a.addEventListener('click', () => {
      header.classList.remove('mobile-open');
      hamburger.setAttribute('aria-expanded', 'false');
    });
  });
}

// Reveal on scroll
const observer = new IntersectionObserver((entries) => {
  entries.forEach((entry) => {
    if (entry.isIntersecting) {
      entry.target.classList.add('in-view');
      observer.unobserve(entry.target);
    }
  });
}, { threshold: 0.12 });

document.querySelectorAll('.fade-up').forEach(el => observer.observe(el));

// Fake input handling for CTA
const ctaForm = document.querySelector('.cta-form');
if (ctaForm) {
  ctaForm.addEventListener('submit', () => {
    const input = ctaForm.querySelector('input[type="email"]');
    if (!input) return;
    const email = input.value.trim();
    if (email) {
      alert('Thanks! We\'ll notify you when Nagardarpan launches.');
      input.value = '';
    }
  });
}

// Hero random mesh network (green dots connected by green lines)
(() => {
  const canvas = document.querySelector('.mesh-canvas');
  if (!canvas) return;
  const ctx = canvas.getContext('2d');
  let nodes = [];
  let width = 0, height = 0, dpr = Math.max(1, window.devicePixelRatio || 1);

  function resize() {
    const rect = canvas.getBoundingClientRect();
    width = Math.floor(rect.width);
    height = Math.floor(rect.height);
    canvas.width = Math.floor(width * dpr);
    canvas.height = Math.floor(height * dpr);
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    initNodes();
  }

  function initNodes() {
    const area = width * height;
    const density = 0.00006; // nodes per px
    const count = Math.max(18, Math.min(120, Math.floor(area * density)));
    nodes = Array.from({ length: count }, () => ({
      x: Math.random() * width,
      y: Math.random() * height,
      vx: (Math.random() - 0.5) * 0.18,
      vy: (Math.random() - 0.5) * 0.18,
      r: 1 + Math.random() * 1.6,
      alert: Math.random() < 0.12 // ~12% red alert nodes
    }));
  }

  const LINE_COLOR = 'rgba(67,255,136,0.18)';
  const DOT_COLOR = 'rgba(67,255,136,0.9)';
  const GLOW_COLOR = 'rgba(67,255,136,0.55)';
  const LINE_ALERT = 'rgba(255,107,107,0.22)';
  const DOT_ALERT = 'rgba(255,107,107,0.95)';
  const GLOW_ALERT = 'rgba(255,107,107,0.55)';

  function step() {
    ctx.clearRect(0, 0, width, height);

    // Connect near neighbors
    const linkDist = Math.max(70, Math.min(130, Math.min(width, height) * 0.22));
    for (let i = 0; i < nodes.length; i++) {
      for (let j = i + 1; j < nodes.length; j++) {
        const a = nodes[i], b = nodes[j];
        const dx = a.x - b.x, dy = a.y - b.y;
        const dist = Math.hypot(dx, dy);
        if (dist < linkDist) {
          const alpha = 1 - dist / linkDist;
          const anyAlert = a.alert || b.alert;
          const base = anyAlert ? 0.22 : 0.18;
          const color = anyAlert ? '255,107,107' : '67,255,136';
          ctx.strokeStyle = `rgba(${color},${base * alpha})`;
          ctx.lineWidth = 1;
          ctx.beginPath();
          ctx.moveTo(a.x, a.y);
          ctx.lineTo(b.x, b.y);
          ctx.stroke();
        }
      }
    }

    // Ensure global connectivity with a lightweight MST backbone
    drawMST();

    // Draw dots with glow
    for (const n of nodes) {
      ctx.fillStyle = n.alert ? GLOW_ALERT : GLOW_COLOR;
      ctx.beginPath();
      ctx.arc(n.x, n.y, n.r * 3, 0, Math.PI * 2);
      ctx.fill();
      ctx.fillStyle = n.alert ? DOT_ALERT : DOT_COLOR;
      ctx.beginPath();
      ctx.arc(n.x, n.y, n.r, 0, Math.PI * 2);
      ctx.fill();
    }

    // Update positions (slow drift)
    for (const n of nodes) {
      n.x += n.vx;
      n.y += n.vy;
      if (n.x < -10) n.x = width + 10; else if (n.x > width + 10) n.x = -10;
      if (n.y < -10) n.y = height + 10; else if (n.y > height + 10) n.y = -10;
    }

    requestAnimationFrame(step);
  }

  // Respect prefers-reduced-motion by freezing animation at a static layout
  const media = window.matchMedia('(prefers-reduced-motion: reduce)');
  function start() {
    resize();
    if (media.matches) {
      // Draw once without animation
      ctx.clearRect(0, 0, width, height);
      const linkDist = Math.max(70, Math.min(130, Math.min(width, height) * 0.22));
      for (let i = 0; i < nodes.length; i++) {
        for (let j = i + 1; j < nodes.length; j++) {
          const a = nodes[i], b = nodes[j];
          const dx = a.x - b.x, dy = a.y - b.y;
          const dist = Math.hypot(dx, dy);
          if (dist < linkDist) {
            const alpha = 1 - dist / linkDist;
            const anyAlert = a.alert || b.alert;
            const base = anyAlert ? 0.22 : 0.18;
            const color = anyAlert ? '255,107,107' : '67,255,136';
            ctx.strokeStyle = `rgba(${color},${base * alpha})`;
            ctx.lineWidth = 1;
            ctx.beginPath();
            ctx.moveTo(a.x, a.y);
            ctx.lineTo(b.x, b.y);
            ctx.stroke();
          }
        }
      }
      for (const n of nodes) {
        ctx.fillStyle = n.alert ? GLOW_ALERT : GLOW_COLOR;
        ctx.beginPath();
        ctx.arc(n.x, n.y, n.r * 3, 0, Math.PI * 2);
        ctx.fill();
        ctx.fillStyle = n.alert ? DOT_ALERT : DOT_COLOR;
        ctx.beginPath();
        ctx.arc(n.x, n.y, n.r, 0, Math.PI * 2);
        ctx.fill();
      }
    } else {
      requestAnimationFrame(step);
    }
  }

  function drawEdge(a, b, strength = 0.24) {
    const anyAlert = a.alert || b.alert;
    const color = anyAlert ? '255,107,107' : '67,255,136';
    ctx.strokeStyle = `rgba(${color},${strength})`;
    ctx.lineWidth = 1;
    ctx.beginPath();
    ctx.moveTo(a.x, a.y);
    ctx.lineTo(b.x, b.y);
    ctx.stroke();
  }

  // Prim's algorithm to create a simple MST ensuring all nodes are connected
  function drawMST() {
    const n = nodes.length;
    if (n <= 1) return;
    const inTree = Array(n).fill(false);
    inTree[0] = true;
    const dist = Array(n).fill(Infinity);
    const parent = Array(n).fill(-1);
    for (let i = 1; i < n; i++) {
      const dx = nodes[i].x - nodes[0].x;
      const dy = nodes[i].y - nodes[0].y;
      dist[i] = dx * dx + dy * dy;
      parent[i] = 0;
    }
    for (let k = 1; k < n; k++) {
      let best = -1, bestDist = Infinity;
      for (let i = 0; i < n; i++) if (!inTree[i] && dist[i] < bestDist) { bestDist = dist[i]; best = i; }
      if (best === -1) break;
      inTree[best] = true;
      drawEdge(nodes[best], nodes[parent[best]], 0.26);
      for (let j = 0; j < n; j++) if (!inTree[j]) {
        const dx = nodes[j].x - nodes[best].x;
        const dy = nodes[j].y - nodes[best].y;
        const d2 = dx * dx + dy * dy;
        if (d2 < dist[j]) { dist[j] = d2; parent[j] = best; }
      }
    }
  }

  window.addEventListener('resize', resize);
  start();
})();

// Login Modal
const loginModal = document.getElementById('login-modal');
const loginBtn = document.getElementById('login-btn');
const loginBtnMobile = document.getElementById('login-btn-mobile');
const loginClose = document.querySelector('.login-close');
const loginCards = document.querySelectorAll('.login-card');

function openLoginModal() {
  if (loginModal) {
    loginModal.classList.add('active');
    loginModal.setAttribute('aria-hidden', 'false');
    document.body.style.overflow = 'hidden';
  }
}

function closeLoginModal() {
  if (loginModal) {
    loginModal.classList.remove('active');
    loginModal.setAttribute('aria-hidden', 'true');
    document.body.style.overflow = '';
  }
}

if (loginBtn) {
  loginBtn.addEventListener('click', (e) => {
    e.preventDefault();
    openLoginModal();
  });
}

if (loginBtnMobile) {
  loginBtnMobile.addEventListener('click', (e) => {
    e.preventDefault();
    header?.classList.remove('mobile-open');
    hamburger?.setAttribute('aria-expanded', 'false');
    openLoginModal();
  });
}

if (loginClose) {
  loginClose.addEventListener('click', closeLoginModal);
}

if (loginModal) {
  loginModal.addEventListener('click', (e) => {
    if (e.target === loginModal || e.target.classList.contains('login-overlay')) {
      closeLoginModal();
    }
  });
}

// Handle login card clicks
loginCards.forEach(card => {
  card.addEventListener('click', () => {
    const role = card.getAttribute('data-role');
    const roleNames = {
      municipal: { name: 'Municipal', url: 'municipal-login.html' },
      ward: { name: 'Ward', url: 'municipal-login.html' },
      citizen: { name: 'Citizen', url: 'citizen-login-page.html' }
    };
    
    const roleInfo = roleNames[role];
    if (roleInfo && roleInfo.url) {
      window.location.href = roleInfo.url;
    } else {
      alert(`${roleInfo?.name || role} login selected. Redirecting to login page...`);
    }
  });
});

// Close modal on Escape key
document.addEventListener('keydown', (e) => {
  if (e.key === 'Escape' && loginModal?.classList.contains('active')) {
    closeLoginModal();
  }
});


