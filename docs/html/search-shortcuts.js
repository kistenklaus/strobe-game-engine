(function() {
  "use strict";

  function searchField() {
    return document.getElementById("MSearchField");
  }

  function focusSearch() {
    const field = searchField();
    if (!field) return;

    field.focus();
    field.select();
  }

  function openFirstResult(attempt) {
    const results = document.getElementById("MSearchResultsWindow");
    const link = results && results.querySelector(
      ".SRResult a, .searchresult a, a[href]"
    );

    if (link) {
      link.click();
      return;
    }

    // Search results are populated asynchronously by Doxygen.
    if (attempt < 20) {
      window.setTimeout(function() {
        openFirstResult(attempt + 1);
      }, 50);
    }
  }

  document.addEventListener("keydown", function(event) {
    console.log("event")
    const target = event.target;
    const isTyping = target instanceof HTMLInputElement ||
      target instanceof HTMLTextAreaElement ||
      target.isContentEditable;

    // / focuses the search field from anywhere on the page.
    if (event.ctrlKey && event.key.toLowerCase() === "f") {
      event.preventDefault();
      event.stopPropagation();
      focusSearch();
      return;
    }

    // Let Doxygen perform the search, then open its first result.
    if (target === searchField() && event.key === "Enter") {
      window.setTimeout(function() {
        openFirstResult(0);
      }, 100);
    }
  });
})();
