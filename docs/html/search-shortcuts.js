(function () {
  "use strict";

  let openBestResult = false;
  let installed = false;

  function searchField() {
    return document.getElementById("MSearchField");
  }

  function focusSearch() {
    const field = searchField();
    if (!field) return;

    field.focus();
    field.select();
  }

  function installSearchHook() {
    if (installed) return true;

    if (typeof searchBox === "undefined" ||
        typeof searchResults === "undefined") {
      return false;
    }

    const originalSearchResults = searchResults.Search;

    searchResults.Search = function (query) {
      const result = originalSearchResults.call(this, query);

      if (openBestResult) {
        openBestResult = false;

        const firstResult = this.NavNext(0);

        if (firstResult) {
          firstResult.click();
        }
      }

      return result;
    };

    installed = true;
    return true;
  }

  function searchAndOpenBestResult() {
    if (!installSearchHook()) {
      window.setTimeout(searchAndOpenBestResult, 25);
      return;
    }

    openBestResult = true;

    // Calls Doxygen's actual search engine immediately. This also loads
    // the appropriate search-index shard if it has not been loaded yet.
    searchBox.Search();
  }

  document.addEventListener("keydown", function (event) {
    const target = event.target;
    const isTyping =
      target instanceof HTMLInputElement ||
      target instanceof HTMLTextAreaElement ||
      target.isContentEditable;

    if (!isTyping && event.key === "/") {
      event.preventDefault();
      focusSearch();
      return;
    }

    if (target === searchField() && event.key === "Enter") {
      event.preventDefault();
      event.stopPropagation();
      searchAndOpenBestResult();
    }
  });
})();
